////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Copyright (C) 2011-2015, Armory Technologies, Inc.                        //
//  Distributed under the GNU Affero General Public License (AGPL v3)         //
//  See LICENSE-ATI or http://www.gnu.org/licenses/agpl.html                  //
//                                                                            //
//                                                                            //
//  Copyright (C) 2016-2018, goatpig                                          //            
//  Distributed under the MIT license                                         //
//  See LICENSE-MIT or https://opensource.org/licenses/MIT                    //                                   
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "ZeroConf.h"
#include "BlockDataMap.h"
#include "ArmoryErrors.h"

using namespace std;
using namespace ArmoryThreading;

#define ZC_GETDATA_TIMEOUT_MS 60000

///////////////////////////////////////////////////////////////////////////////
ZcPreprocessPacket::~ZcPreprocessPacket()
{}

///////////////////////////////////////////////////////////////////////////////
ZcGetPacket::~ZcGetPacket()
{}

///////////////////////////////////////////////////////////////////////////////
//ZeroConfContainer Methods
///////////////////////////////////////////////////////////////////////////////
Tx ZeroConfContainer::getTxByHash(const BinaryData& txHash) const
{
   auto ss = getSnapshot();
   if (ss == nullptr)
      return Tx();

   auto& txhashmap = ss->txHashToDBKey_;
   const auto keyIter = txhashmap.find(txHash);

   if (keyIter == txhashmap.end())
      return Tx();

   auto& txmap = ss->txMap_;
   auto txiter = txmap.find(keyIter->second);

   if (txiter == txmap.end())
      return Tx();

   auto txCopy = txiter->second->tx_;
   
   //get zc outpoints id
   for (unsigned i=0; i<txCopy.getNumTxIn(); i++)
   {
      auto&& txin = txCopy.getTxInCopy(i);
      auto&& op = txin.getOutPoint();

      auto opIter = txhashmap.find(op.getTxHashRef());
      if (opIter == txhashmap.end())
      {
         txCopy.pushBackOpId(0);
         continue;
      }

      BinaryRefReader brr(opIter->second);
      brr.advance(2);
      txCopy.pushBackOpId(brr.get_uint32_t(BE));
   }

   return txCopy;
}

///////////////////////////////////////////////////////////////////////////////
bool ZeroConfContainer::hasTxByHash(const BinaryData& txHash) const
{
   auto ss = getSnapshot();
   auto& txhashmap = ss->txHashToDBKey_;
   return (txhashmap.find(txHash) != txhashmap.end());
}

///////////////////////////////////////////////////////////////////////////////
bool ZeroConfContainer::purge(
   const Blockchain::ReorganizationState& reorgState,
   shared_ptr<ZeroConfSharedStateSnapshot> ss,
   map<BinaryData, BinaryData>& minedKeys)
{
   if (db_ == nullptr || ss->txMap_.size() == 0)
      return true;

   set<BinaryData> keysToDelete;
   auto& zcMap = ss->txMap_;
   auto& txoutspentbyzc = ss->txOutsSpentByZC_;

   auto updateChildren = [&zcMap, &minedKeys, &txoutspentbyzc, this](
      BinaryDataRef& txHash, const BinaryData& blockKey,
      map<BinaryData, unsigned> minedHashes)->void
   {
      auto spentIter = outPointsSpentByKey_.find(txHash);
      if (spentIter == outPointsSpentByKey_.end())
         return;

      //is this zc mined or just invalidated?
      auto minedIter = minedHashes.find(txHash);
      if (minedIter == minedHashes.end())
         return;
      auto txid = minedIter->second;

      //list children by key
      set<BinaryDataRef> keysToClear;
      for (auto& op_pair : spentIter->second)
         keysToClear.insert(op_pair.second);

      //run through children, replace key
      for (auto& zckey : keysToClear)
      {
         auto zcIter = zcMap.find(zckey);
         if (zcIter == zcMap.end())
            continue;

         for (auto& input : zcIter->second->inputs_)
         {
            if (input.opRef_.getTxHashRef() != txHash)
               continue;

            auto prevKey = input.opRef_.getDbKey();
            txoutspentbyzc.erase(prevKey);
            input.opRef_.reset();

            BinaryWriter bw_key(8);
            bw_key.put_BinaryData(blockKey);
            bw_key.put_uint16_t(txid, BE);
            bw_key.put_uint16_t(input.opRef_.getIndex(), BE);

            minedKeys.insert(make_pair(prevKey, bw_key.getData()));
            input.opRef_.getDbKey() = bw_key.getData();

            zcIter->second->isChainedZc_ = false;
            zcIter->second->needsReparsed_ = true;
         }
      }
   };

   //lambda to purge zc map per block
   auto purgeZcMap =
      [&zcMap, &keysToDelete, &reorgState, &minedKeys, this, updateChildren](
         map<BinaryDataRef, set<unsigned>>& spentOutpoints,
         map<BinaryData, unsigned> minedHashes,
         const BinaryData& blockKey)->void
   {
      auto zcMapIter = zcMap.begin();
      while (zcMapIter != zcMap.end())
      {
         auto& zc = zcMapIter->second;
         bool invalidated = false;
         for (auto& input : zc->inputs_)
         {
            auto opIter = spentOutpoints.find(input.opRef_.getTxHashRef());
            if (opIter == spentOutpoints.end())
               continue;

            auto indexIter = opIter->second.find(input.opRef_.getIndex());
            if (indexIter == opIter->second.end())
               continue;

            //the outpoint for this zc is spent, invalidate the zc
            invalidated = true;
            break;
         }

         if (invalidated)
         {
            //mark for deletion
            keysToDelete.insert(zcMapIter->first);

            //this zc is now invalid, process its children
            auto&& zchash = zcMapIter->second->getTxHash().getRef();
            updateChildren(zchash, blockKey, minedHashes);
         }

         ++zcMapIter;
      }
   };

   if (!reorgState.prevTopStillValid_)
   {
      //reset resolved outpoints cause of reorg
      for (auto& zc_pair : zcMap)
         zc_pair.second->reset();
   }

   auto getIdSpoofLbd = [](const BinaryData&)->unsigned
   {
      return 0;
   };

   //get all txhashes for the new blocks
   ZcUpdateBatch batch;
   auto bcPtr = db_->blockchain();
   try
   {
      auto currentHeader = reorgState.prevTop_;
      if (!reorgState.prevTopStillValid_)
         currentHeader = reorgState.reorgBranchPoint_;

      //get the next header
      currentHeader = bcPtr->getHeaderByHash(currentHeader->getNextHash());

      //loop over headers
      while (currentHeader != nullptr)
      {
         //grab block
         auto&& rawBlock = db_->getRawBlock(
            currentHeader->getBlockHeight(),
            currentHeader->getDuplicateID());

         BlockData block;
         block.deserialize(
            rawBlock.getPtr(), rawBlock.getSize(),
            currentHeader, getIdSpoofLbd,
            false, false);

         //build up hash set
         map<BinaryDataRef, set<unsigned>> spentOutpoints;
         map<BinaryData, unsigned> minedHashes;
         auto txns = block.getTxns();
         for (unsigned txid = 0; txid < txns.size(); txid++)
         {
            auto& txn = txns[txid];
            for (unsigned iin = 0; iin < txn->txins_.size(); iin++)
            {
               auto txInRef = txn->getTxInRef(iin);
               BinaryRefReader brr(txInRef);
               auto hash = brr.get_BinaryDataRef(32);
               auto index = brr.get_uint32_t();

               auto& indexSet = spentOutpoints[hash];
               indexSet.insert(index);
            }

            minedHashes.insert(make_pair(txn->getHash(), txid));
         }

         purgeZcMap(spentOutpoints, minedHashes,
            currentHeader->getBlockDataKey());

         if (BlockDataManagerConfig::getDbType() != ARMORY_DB_SUPER)
         {
            //purge mined hashes
            for (auto& minedHash : minedHashes)
            {
               allZcTxHashes_.erase(minedHash.first);
               batch.txHashesToDelete_.insert(minedHash.first);
            }
         }

         //next block
         if (currentHeader->getThisHash() == reorgState.newTop_->getThisHash())
            break;

         auto& bhash = currentHeader->getNextHash();
         currentHeader = bcPtr->getHeaderByHash(bhash);
      }
   }
   catch (...)
   {
   }

   if (reorgState.prevTopStillValid_)
   {
      dropZC(ss, keysToDelete);
      return true;
   }
   else
   {
      //reset containers and resolve outpoints anew after a reorg
      reset();
      preprocessZcMap(zcMap);

      //delete keys from DB
      batch.keysToDelete_ = move(keysToDelete);
      auto fut = batch.getCompletedFuture();
      updateBatch_.push_back(move(batch));
      fut.wait();

      return false;
   }
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::preprocessZcMap(
   map<BinaryDataRef, shared_ptr<ParsedTx>>& zcMap)
{
   //run threads to preprocess the zcMap
   auto counter = make_shared<atomic<unsigned>>();
   counter->store(0, memory_order_relaxed);

   vector<shared_ptr<ParsedTx>> txVec;
   txVec.reserve(zcMap.size());
   for (auto& txPair : zcMap)
      txVec.push_back(txPair.second);

   auto parserLdb = [this, &txVec, counter](void)->void
   {
      while (1)
      {
         auto id = counter->fetch_add(1, memory_order_relaxed);
         if (id >= txVec.size())
            return;

         auto txIter = txVec.begin() + id;
         this->preprocessTx(*(*txIter));
      }
   };

   vector<thread> parserThreads;
   for (unsigned i = 1; i < MAX_THREADS(); i++)
      parserThreads.push_back(thread(parserLdb));
   parserLdb();

   for (auto& thr : parserThreads)
   {
      if (thr.joinable())
         thr.join();
   }
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::reset()
{
   keyToSpentScrAddr_.clear();
   outPointsSpentByKey_.clear();
   keyToFundedScrAddr_.clear();
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::dropZC(
   shared_ptr<ZeroConfSharedStateSnapshot> ss, const BinaryDataRef& key)
{
   auto& txMap = ss->txMap_;
   auto& txioMap = ss->txioMap_;
   auto& txOuts = ss->txOutsSpentByZC_;

   auto iter = txMap.find(key);
   if (iter == txMap.end())
      return;

   /*** lambas ***/
   auto dropTxios = [&txioMap](
      const BinaryDataRef zcKey,
      const BinaryDataRef scrAddr)->void
   {
      auto mapIter = txioMap.find(scrAddr);
      if (mapIter == txioMap.end())
         return;

      map<BinaryData, shared_ptr<TxIOPair>> revisedTxioMap;
      auto& txios = mapIter->second;
      for (auto& txio_pair : txios)
      {
         //if the txio is keyed by our zc, do not keep it
         if (txio_pair.first.startsWith(zcKey))
            continue;

         //otherwise, it should have a txin
         if (!txio_pair.second->hasTxIn())
            continue;

         //at this point the txin is ours. if the txout is not zc, drop the txio
         if (!txio_pair.second->hasTxOutZC())
            continue;

         //wipe our txin from the txio, keep the txout as it belongs to another zc
         auto txio = make_shared<TxIOPair>(*txio_pair.second);
         txio->setTxIn(BinaryData());
         revisedTxioMap.insert(move(make_pair(txio_pair.first, move(txio))));
      }

      if (revisedTxioMap.size() == 0)
      {
         txioMap.erase(mapIter);
         return;
      }

      mapIter->second = move(revisedTxioMap);
   };

   /*** drop tx from snapshot ***/
   auto&& hashToDelete = iter->second->getTxHash().getRef();
   ss->txHashToDBKey_.erase(hashToDelete);

   //drop from outPointsSpentByKey_
   outPointsSpentByKey_.erase(hashToDelete);
   for (auto& input : iter->second->inputs_)
   {
      auto opIter = 
         outPointsSpentByKey_.find(input.opRef_.getTxHashRef());
      if (opIter == outPointsSpentByKey_.end())
         continue;

      //erase the index
      opIter->second.erase(input.opRef_.getIndex());

      //erase the txhash if the index map is empty
      if (opIter->second.size() == 0)
      {
         minedTxHashes_.erase(opIter->first);
         outPointsSpentByKey_.erase(opIter);
      }
   }

   //drop from keyToSpendScrAddr_
   auto saSetIter = keyToSpentScrAddr_.find(key);
   if (saSetIter != keyToSpentScrAddr_.end())
   {
      for (auto& sa : *saSetIter->second)
      {
         BinaryData sabd(sa);
         dropTxios(key, sa);
      }
      keyToSpentScrAddr_.erase(saSetIter);
   }

   //drop from keyToFundedScrAddr_
   auto fundedIter = keyToFundedScrAddr_.find(key);
   if (fundedIter != keyToFundedScrAddr_.end())
   {
      for (auto& sa : fundedIter->second)
      {
         BinaryData sabd(sa);
         dropTxios(key, sa);
      }

      keyToFundedScrAddr_.erase(fundedIter);
   }

   //drop from txOutsSpentByZC_
   for (auto& input : iter->second->inputs_)
   {
      if (!input.isResolved())
         continue;
      txOuts.erase(input.opRef_.getDbKey());
   }

   //delete tx
   txMap.erase(iter);
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::dropZC(
   shared_ptr<ZeroConfSharedStateSnapshot> ss, const set<BinaryData>& zcKeys)
{
   if (zcKeys.size() == 0)
      return;

   auto rIter = zcKeys.rbegin();
   while (rIter != zcKeys.rend())
      dropZC(ss, *rIter++);

   ZcUpdateBatch batch;
   batch.keysToDelete_ = zcKeys;
   updateBatch_.push_back(move(batch));
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::parseNewZC(ZcActionStruct zcAction)
{
   bool notify = true;
   map<BinaryData, BinaryData> previouslyValidKeys;
   map<BinaryData, BinaryData> minedKeys;
   auto ss = ZeroConfSharedStateSnapshot::copy(snapshot_);
   map<BinaryDataRef, shared_ptr<ParsedTx>> zcMap;
   map<BinaryData, WatcherTxBody> watcherMap;
   pair<string, string> requestor;

   switch (zcAction.action_)
   {
   case Zc_Purge:
   {
      //build set of currently valid keys
      for (auto& txpair : ss->txMap_)
      {
         previouslyValidKeys.emplace(
            make_pair(txpair.first, txpair.second->getTxHash()));
      }

      //purge mined zc
      auto result = purge(zcAction.reorgState_, ss, minedKeys);
      notify = false;

      //setup batch with all tracked zc
      if (zcAction.batch_ == nullptr)
         zcAction.batch_ = make_shared<ZeroConfBatch>(false);
      zcAction.batch_->zcMap_ = ss->txMap_;
      zcAction.batch_->isReadyPromise_->set_value(ArmoryErrorCodes::Success);

      if (!result)
      {
         reset();
         ss = nullptr;
      }
   }

   case Zc_NewTx:
   {
      try
      {      
         auto batchTxMap = move(getBatchTxMap(zcAction.batch_, ss));
         zcMap = move(batchTxMap.txMap_);
         watcherMap = move(batchTxMap.watcherMap_);
         requestor = move(batchTxMap.requestor_);
      }
      catch (ZcBatchError&)
      {
         return;
      }

      break;
   }

   case Zc_Shutdown:
   {
      reset();
      return;
   }

   default:
      return;
   }

   parseNewZC(move(zcMap), ss, true, notify, requestor, watcherMap);
   if (zcAction.resultPromise_ != nullptr)
   {
      auto purgePacket = make_shared<ZcPurgePacket>();
      purgePacket->minedTxioKeys_ = move(minedKeys);

      for (auto& wasValid : previouslyValidKeys)
      {
         auto keyIter = snapshot_->txMap_.find(wasValid.first);
         if (keyIter != snapshot_->txMap_.end())
            continue;

         purgePacket->invalidatedZcKeys_.insert(wasValid);
      }

      zcAction.resultPromise_->set_value(purgePacket);
   }

   if (watcherMap.empty())
      return;
   
   //process duplicate broadcast requests
   for (auto& watcherObj : watcherMap)
   {
      if (watcherObj.second.extraRequestors_.empty())
         continue;

      auto keyIter = snapshot_->txHashToDBKey_.find(watcherObj.first);
      if (keyIter == snapshot_->txHashToDBKey_.end())
      {
         //tx was not added to mempool, skip
         continue;
      }

      //tx was added to mempool, report already-in-mempool error to 
      //duplicate requestors
      for (auto& extra : watcherObj.second.extraRequestors_)
      {
         bdvCallbacks_->pushZcError(extra.second, watcherObj.first, 
            ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool, 
            "Extra requestor broadcast error: Already in mempool",
            extra.first);
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::parseNewZC(
   map<BinaryDataRef, shared_ptr<ParsedTx>> zcMap,
   shared_ptr<ZeroConfSharedStateSnapshot> ss,
   bool updateDB, bool notify,
   const pair<string, string>& requestor,
   std::map<BinaryData, WatcherTxBody>& watcherMap)
{
   unique_lock<mutex> lock(parserMutex_);
   ZcUpdateBatch batch;

   auto iter = zcMap.begin();
   while (iter != zcMap.end())
   {
      if (iter->second->status() == Tx_Mined ||
         iter->second->status() == Tx_Invalid || 
         iter->second->status() == Tx_Skip)
         zcMap.erase(iter++);
      else
         ++iter;
   }

   if (ss == nullptr)
      ss = make_shared<ZeroConfSharedStateSnapshot>();

   auto& txmap = ss->txMap_;
   auto& txhashmap = ss->txHashToDBKey_;
   auto& txoutsspentbyzc = ss->txOutsSpentByZC_;
   auto& txiomap = ss->txioMap_;

   for (auto& newZCPair : zcMap)
   {
      if (BlockDataManagerConfig::getDbType() != ARMORY_DB_SUPER)
      {
         auto& txHash = newZCPair.second->getTxHash();
         auto insertIter = allZcTxHashes_.insert(txHash);
         if (!insertIter.second)
            continue;
      }
      else
      {
         if (txmap.find(newZCPair.first) != txmap.end())
            continue;
      }

      batch.zcToWrite_.insert(newZCPair);
   }

   map<BinaryDataRef, shared_ptr<ParsedTx>> invalidatedTx;

   bool hasChanges = false;

   map<string, ParsedZCData> flaggedBDVs;

   //zckey fetch lambda
   auto getzckeyfortxhash = [&txhashmap]
   (const BinaryData& txhash, BinaryData& zckey_output)->bool
   {
      auto global_iter = txhashmap.find(txhash);
      if (global_iter == txhashmap.end())
         return false;

      zckey_output = global_iter->second;
      return true;
   };

   //zc tx fetch lambda
   auto getzctxforkey = [&txmap]
   (const BinaryData& zc_key)->const ParsedTx&
   {
      auto global_iter = txmap.find(zc_key);
      if (global_iter == txmap.end())
         throw runtime_error("no zc tx for this key");

      return *global_iter->second;
   };

   function<set<BinaryData>(const BinaryData&)> getTxChildren =
      [&](const BinaryData& zcKey)->set<BinaryData>
   {
      set<BinaryData> childKeys;
      BinaryDataRef txhash;

      try
      {
         auto& parsedTx = getzctxforkey(zcKey);
         txhash = parsedTx.getTxHash().getRef();
      }
      catch (exception&)
      {
         return childKeys;
      }

      auto spentOP_iter = outPointsSpentByKey_.find(txhash);
      if (spentOP_iter != outPointsSpentByKey_.end())
      {
         auto& keymap = spentOP_iter->second;

         for (auto& keypair : keymap)
         {
            auto&& childrenKeys = getTxChildren(keypair.second);
            childKeys.insert(move(keypair.second));
            for (auto& c_key : childrenKeys)
               childKeys.insert(move(c_key));
         }
      }

      return childKeys;
   };

   //zc logic
   set<BinaryDataRef> addedZcKeys;
   for (auto& newZCPair : zcMap)
   {
      auto&& txHash = newZCPair.second->getTxHash().getRef();
      if (txhashmap.find(txHash) != txhashmap.end())
      {
         /*
         Already have this ZC, why is it passed for parsing again?
         Most common reason for reparsing is a child zc which parent's
         was mined (outpoint now resolves to a dbkey instead of 
         the previous zckey)
         */

         if (!newZCPair.second->needsReparsed_)
         {
            //tx wasn't flagged as needing processed again, skip it
            continue;
         }

         //turn off reparse flag
         newZCPair.second->needsReparsed_ = false;
      }

      {
         auto&& bulkData = ZCisMineBulkFilter(
            *newZCPair.second, newZCPair.first,
            getzckeyfortxhash, getzctxforkey);

         //check for replacement
         {
            //loop through all outpoints consumed by this ZC
            for (auto& idSet : bulkData.outPointsSpentByKey_)
            {
               set<BinaryData> childKeysToDrop;

               //compare them to the list of currently spent outpoints
               auto hashIter = outPointsSpentByKey_.find(idSet.first);
               if (hashIter == outPointsSpentByKey_.end())
                  continue;

               for (auto opId : idSet.second)
               {
                  auto idIter = hashIter->second.find(opId.first);
                  if (idIter != hashIter->second.end())
                  {
                     try
                     {
                        //gather replaced tx children
                        auto&& keySet = getTxChildren(idIter->second);
                        keySet.insert(idIter->second);
                        hasChanges = true;

                        //drop the replaced transactions
                        for (auto& key : keySet)
                        {
                           auto txiter = txmap.find(key);
                           if (txiter != txmap.end())
                              invalidatedTx.insert(*txiter);
                           childKeysToDrop.insert(key);
                        }
                     }
                     catch (exception&)
                     {
                        continue;
                     }
                  }
               }

               auto rIter = childKeysToDrop.rbegin();
               while (rIter != childKeysToDrop.rend())
                  dropZC(ss, *rIter++);
            }
         }

         //add ZC if its relevant
         if (newZCPair.second->status() != Tx_Invalid &&
            newZCPair.second->status() != Tx_Uninitialized &&
            !bulkData.isEmpty())
         {       
            addedZcKeys.insert(newZCPair.first);
            hasChanges = true;

            //merge spent outpoints
            txoutsspentbyzc.insert(
               bulkData.txOutsSpentByZC_.begin(),
               bulkData.txOutsSpentByZC_.end());

            /***
            The outpoint spender map structure is as follow:
            map<txhash-of-output-owner, map<output-id, txhash-of-spender>>

            The hash of the owner and the hash of the spender are BinaryDataRef.
            When ZCisMineBulkFilter parses new zc, it references the owner hash
            from its own outpoints.

            When we merge the spender data with the snapshot's spender map, we
            want to reference the owner hash directly from the relevant ParsedTx 
            object instead. 
            
            This prevents the need to rekey the owner hash if the spender expires
            before the owner.
            ***/

            for (auto& idmap : bulkData.outPointsSpentByKey_)
            {
               pair<BinaryDataRef, map<unsigned, BinaryDataRef>> outpoints;

               //is this owner hash already in the map?
               auto ownerIter = outPointsSpentByKey_.find(idmap.first);
               if (ownerIter == outPointsSpentByKey_.end())
               {
                  BinaryDataRef ownerHash;

                  //missing this owner, look for it in the zc map
                  auto zcKeyIter = ss->txHashToDBKey_.find(idmap.first);
                  if (zcKeyIter != ss->txHashToDBKey_.end())
                  {
                     //txHashToDBKey_ references the owner hash, we can use it as is
                     ownerHash = zcKeyIter->first;
                  }

                  if (ownerHash.getSize() == 0)
                  {
                     //could not find a zc owner for this hash, most likely belongs
                     //to a mined tx
                     auto minedHashIter = minedTxHashes_.insert(idmap.first);
                     ownerHash = minedHashIter.first->getRef();
                  }

                  //insert the key in the spender map
                  ownerIter = outPointsSpentByKey_.emplace(
                     ownerHash, map<unsigned, BinaryDataRef>()).first;
               }

               //update spender map
               ownerIter->second.insert(idmap.second.begin(), idmap.second.end());
            }

            //merge scrAddr spent by key
            for (auto& sa_pair : bulkData.keyToSpentScrAddr_)
            {
               auto insertResult = keyToSpentScrAddr_.insert(sa_pair);
               if (insertResult.second == false)
                  insertResult.first->second = move(sa_pair.second);
            }

            //merge scrAddr funded by key
            typedef map<BinaryDataRef, set<BinaryDataRef>>::iterator mapbd_setbd_iter;
            keyToFundedScrAddr_.insert(
               move_iterator<mapbd_setbd_iter>(bulkData.keyToFundedScrAddr_.begin()),
               move_iterator<mapbd_setbd_iter>(bulkData.keyToFundedScrAddr_.end()));

            //merge new txios
            txhashmap[txHash] = newZCPair.first;
            txmap[newZCPair.first] = newZCPair.second;

            for (auto& saTxio : bulkData.scrAddrTxioMap_)
            {
               auto saIter = txiomap.find(saTxio.first);
               if (saIter != txiomap.end())
               {
                  for (auto& newTxio : saTxio.second)
                  {
                     auto insertIter = saIter->second.insert(newTxio);
                     if (insertIter.second == false)
                        insertIter.first->second = newTxio.second;
                  }
               }
               else
               {
                  txiomap.insert(move(saTxio));
               }
            }

            //flag affected BDVs
            for (auto& bdvMap : bulkData.flaggedBDVs_)
            {
               auto& parserResult = flaggedBDVs[bdvMap.first];
               parserResult.mergeTxios(bdvMap.second);
            }
         }
      }
   }

   if (updateDB && batch.hasData())
   {
      //post new zc for writing to db, no need to wait on it
      updateBatch_.push_back(move(batch));
   }

   //find BDVs affected by invalidated keys
   if (invalidatedTx.size() > 0)
   {
      //TODO: multi thread this at some point

      for (auto& tx_pair : invalidatedTx)
      {
         //gather all scrAddr from invalidated tx
         set<BinaryDataRef> addrRefs;

         for (auto& input : tx_pair.second->inputs_)
         {
            if (!input.isResolved())
               continue;

            addrRefs.insert(input.scrAddr_.getRef());
         }

         for (auto& output : tx_pair.second->outputs_)
            addrRefs.insert(output.scrAddr_.getRef());

         //flag relevant BDVs
         for (auto& addrRef : addrRefs)
         {
            auto&& bdvid_set = bdvCallbacks_->hasScrAddr(addrRef);
            for (auto& bdvid : bdvid_set)
            {
               auto& bdv = flaggedBDVs[bdvid];
               bdv.invalidatedKeys_.insert(
                  make_pair(tx_pair.first, tx_pair.second->getTxHash()));
               hasChanges = true;
            }
         }
      }
   }

   //swap in new state
   atomic_store_explicit(&snapshot_, ss, memory_order_release);

   //notify bdvs
   if (!hasChanges)
      return;

   if (!notify)
      return;

   //prepare notifications
   auto newZcKeys =
      make_shared<map<BinaryData, shared_ptr<set<BinaryDataRef>>>>();
   for (auto& newKey : addedZcKeys)
   {
      //fill key to spent scrAddr map
      shared_ptr<set<BinaryDataRef>> spentScrAddr = nullptr;
      auto iter = keyToSpentScrAddr_.find(newKey);
      if (iter != keyToSpentScrAddr_.end())
         spentScrAddr = iter->second;

      auto addr_pair = make_pair(newKey, move(spentScrAddr));
      newZcKeys->insert(move(addr_pair));
   }

   bdvCallbacks_->pushZcNotification(
      ss, newZcKeys,
      flaggedBDVs, 
      requestor.first, requestor.second, 
      watcherMap);
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::preprocessTx(ParsedTx& tx) const
{
   auto& txHash = tx.getTxHash();
   auto&& txref = db_->getTxRef(txHash);

   if (txref.isInitialized())
   {
      tx.state_ = Tx_Mined;
      return;
   }

   uint8_t const * txStartPtr = tx.tx_.getPtr();
   unsigned len = tx.tx_.getSize();

   auto nTxIn = tx.tx_.getNumTxIn();
   auto nTxOut = tx.tx_.getNumTxOut();

   //try to resolve as many outpoints as we can. unresolved outpoints are 
   //either invalid or (most likely) children of unconfirmed transactions
   if (nTxIn != tx.inputs_.size())
   {
      tx.inputs_.clear();
      tx.inputs_.resize(nTxIn);
   }

   if (nTxOut != tx.outputs_.size())
   {
      tx.outputs_.clear();
      tx.outputs_.resize(nTxOut);
   }

   for (uint32_t iin = 0; iin < nTxIn; iin++)
   {
      auto& txIn = tx.inputs_[iin];
      if (txIn.isResolved())
         continue;

      auto& opRef = txIn.opRef_;

      if (!opRef.isInitialized())
      {
         auto offset = tx.tx_.getTxInOffset(iin);
         if (offset > len)
            throw runtime_error("invalid txin offset");
         BinaryDataRef inputDataRef(txStartPtr + offset, len - offset);
         opRef.unserialize(inputDataRef);
      }

      if (!opRef.isResolved())
      {
         //resolve outpoint to dbkey
         txIn.opRef_.resolveDbKey(db_);
         if (!opRef.isResolved())
            continue;
      }

      //grab txout
      StoredTxOut stxOut;
      if (!db_->getStoredTxOut(stxOut, opRef.getDbKey()))
         continue;

      if (db_->armoryDbType() == ARMORY_DB_SUPER)
         opRef.getDbKey() = stxOut.getDBKey(false);

      if (stxOut.isSpent())
      {
         tx.state_ = Tx_Invalid;
         return;
      }

      //set txin address and value
      txIn.scrAddr_ = stxOut.getScrAddress();
      txIn.value_ = stxOut.getValue();
   }

   for (uint32_t iout = 0; iout < nTxOut; iout++)
   {
      auto& txOut = tx.outputs_[iout];
      if (txOut.isInitialized())
         continue;

      auto offset = tx.tx_.getTxOutOffset(iout);
      auto len = tx.tx_.getTxOutOffset(iout + 1) - offset;

      BinaryRefReader brr(txStartPtr + offset, len);
      txOut.value_ = brr.get_uint64_t();

      auto scriptLen = brr.get_var_int();
      auto scriptRef = brr.get_BinaryDataRef(scriptLen);
      txOut.scrAddr_ = move(BtcUtils::getTxOutScrAddr(scriptRef));

      txOut.offset_ = offset;
      txOut.len_ = len;
   }

   tx.isRBF_ = tx.tx_.isRBF();


   bool txInResolved = true;
   for (auto& txin : tx.inputs_)
   {
      if (txin.isResolved())
         continue;

      txInResolved = false;
      break;
   }

   if (!txInResolved)
      tx.state_ = Tx_Unresolved;
   else
      tx.state_ = Tx_Resolved;
}

///////////////////////////////////////////////////////////////////////////////
ZeroConfContainer::BulkFilterData ZeroConfContainer::ZCisMineBulkFilter(
   ParsedTx& parsedTx, const BinaryDataRef& ZCkey,
   function<bool(const BinaryData&, BinaryData&)> getzckeyfortxhash,
   function<const ParsedTx&(const BinaryData&)> getzctxforkey)
{
   BulkFilterData bulkData;
   if (parsedTx.status() == Tx_Mined || parsedTx.status() == Tx_Invalid)
      return bulkData;

   auto mainAddressSet = scrAddrMap_->get();

   auto filter = [mainAddressSet, this]
      (const BinaryData& addr)->pair<bool, set<string>>
   {
      pair<bool, set<string>> flaggedBDVs;
      flaggedBDVs.first = false;

      
      //Check if this address is being watched before looking for specific BDVs
      auto addrIter = mainAddressSet->find(addr.getRef());
      if (addrIter == mainAddressSet->end())
      {
         if (BlockDataManagerConfig::getDbType() == ARMORY_DB_SUPER)
         {
            /*
            We got this far because no BDV is watching this address and the DB
            is running as a supernode. In supernode we track all ZC regardless 
            of watch status. Flag as true to process the ZC, but do not attach
            a bdv ID as no clients will be notified of this zc.
            */
            flaggedBDVs.first = true;
         }

         return flaggedBDVs;
      }

      flaggedBDVs.first = true;
      flaggedBDVs.second = move(bdvCallbacks_->hasScrAddr(addr.getRef()));
      return flaggedBDVs;
   };

   auto insertNewZc = [&bulkData](const BinaryData& sa,
      BinaryData txiokey, shared_ptr<TxIOPair> txio,
      set<string> flaggedBDVs, bool consumesTxOut)->void
   {
      if (consumesTxOut)
         bulkData.txOutsSpentByZC_.insert(txiokey);

      auto& key_txioPair = bulkData.scrAddrTxioMap_[sa];
      key_txioPair[txiokey] = move(txio);

      for (auto& bdvId : flaggedBDVs)
         bulkData.flaggedBDVs_[bdvId].scrAddrs_.insert(sa);
   };

   if (parsedTx.status() == Tx_Uninitialized ||
      parsedTx.status() == Tx_ResolveAgain)
      preprocessTx(parsedTx);

   auto& txHash = parsedTx.getTxHash();
   bool isRBF = parsedTx.isRBF_;
   bool isChained = parsedTx.isChainedZc_;

   //if parsedTx has unresolved outpoint, they are most likely ZC
   for (auto& input : parsedTx.inputs_)
   {
      if (input.isResolved())
      {
         //check resolved key is valid
         if (input.opRef_.isZc())
         {
            try
            {
               isChained = true;
               auto& chainedZC = getzctxforkey(input.opRef_.getDbTxKeyRef());
               if (chainedZC.status() == Tx_Invalid)
                  throw runtime_error("invalid parent zc");
            }
            catch (exception&)
            {
               parsedTx.state_ = Tx_Invalid;
               return bulkData;
            }
         }
         else
         {
            auto&& keyRef = input.opRef_.getDbKey().getSliceRef(0, 4);
            auto height = DBUtils::hgtxToHeight(keyRef);
            auto dupId = DBUtils::hgtxToDupID(keyRef);

            if (db_->getValidDupIDForHeight(height) != dupId)
            {
               parsedTx.state_ = Tx_Invalid;
               return bulkData;
            }
         }

         continue;
      }

      auto& opZcKey = input.opRef_.getDbKey();
      if (!getzckeyfortxhash(input.opRef_.getTxHashRef(), opZcKey))
      {
         if (BlockDataManagerConfig::getDbType() == ARMORY_DB_SUPER ||
            allZcTxHashes_.find(input.opRef_.getTxHashRef()) == allZcTxHashes_.end())
            continue;
      }

      isChained = true;

      try
      {
         auto& chainedZC = getzctxforkey(opZcKey);
         auto&& chainedTxOut = chainedZC.tx_.getTxOutCopy(input.opRef_.getIndex());

         input.value_ = chainedTxOut.getValue();
         input.scrAddr_ = chainedTxOut.getScrAddressStr();
         isRBF |= chainedZC.tx_.isRBF();
         input.opRef_.setTime(chainedZC.tx_.getTxTime());

         opZcKey.append(WRITE_UINT16_BE(input.opRef_.getIndex()));
      }
      catch (runtime_error&)
      {
         continue;
      }
   }

   parsedTx.isRBF_ = isRBF;
   parsedTx.isChainedZc_ = isChained;

   //spent txios
   unsigned iin = 0;
   for (auto& input : parsedTx.inputs_)
   {
      auto inputId = iin++;
      if (!input.isResolved())
      {
         if (db_->armoryDbType() == ARMORY_DB_SUPER)
         {
            parsedTx.state_ = Tx_Invalid;
            return bulkData;
         }
         else
         {
            parsedTx.state_ = Tx_ResolveAgain;
         }

         continue;
      }

      //keep track of all outputs this ZC consumes
      auto& id_map = bulkData.outPointsSpentByKey_[input.opRef_.getTxHashRef()];
      id_map.insert(make_pair(input.opRef_.getIndex(), ZCkey));

      auto&& flaggedBDVs = filter(input.scrAddr_);
      if (!isChained && !flaggedBDVs.first)
         continue;

      auto txio = make_shared<TxIOPair>(
         TxRef(input.opRef_.getDbTxKeyRef()), input.opRef_.getIndex(),
         TxRef(ZCkey), inputId);

      txio->setTxHashOfOutput(input.opRef_.getTxHashRef());
      txio->setTxHashOfInput(txHash);
      txio->setValue(input.value_);
      auto tx_time = input.opRef_.getTime();
      if (tx_time == UINT64_MAX)
         tx_time = parsedTx.tx_.getTxTime();
      txio->setTxTime(tx_time);
      txio->setRBF(isRBF);
      txio->setChained(isChained);

      auto&& txioKey = txio->getDBKeyOfOutput();
      insertNewZc(input.scrAddr_, move(txioKey), move(txio),
         move(flaggedBDVs.second), true);

      auto& updateSet = bulkData.keyToSpentScrAddr_[ZCkey];
      if (updateSet == nullptr)
         updateSet = make_shared<set<BinaryDataRef>>();
      updateSet->insert(input.scrAddr_.getRef());
   }

   //funded txios
   unsigned iout = 0;
   for (auto& output : parsedTx.outputs_)
   {
      auto outputId = iout++;

      auto&& flaggedBDVs = filter(output.scrAddr_);
      if (flaggedBDVs.first)
      {
         auto txio = make_shared<TxIOPair>(TxRef(ZCkey), outputId);

         txio->setValue(output.value_);
         txio->setTxHashOfOutput(txHash);
         txio->setTxTime(parsedTx.tx_.getTxTime());
         txio->setUTXO(true);
         txio->setRBF(isRBF);
         txio->setChained(isChained);

         auto& fundedScrAddr = bulkData.keyToFundedScrAddr_[ZCkey];
         fundedScrAddr.insert(output.scrAddr_.getRef());

         auto&& txioKey = txio->getDBKeyOfOutput();
         insertNewZc(output.scrAddr_, move(txioKey),
            move(txio), move(flaggedBDVs.second), false);
      }
   }

   return bulkData;
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::clear()
{
   snapshot_.reset();
}

///////////////////////////////////////////////////////////////////////////////
bool ZeroConfContainer::isTxOutSpentByZC(const BinaryData& dbkey) const
{
   auto ss = getSnapshot();
   if (ss == nullptr)
      return false;

   auto& txoutset = ss->txOutsSpentByZC_;
   if (txoutset.find(dbkey) != txoutset.end())
      return true;

   return false;
}

///////////////////////////////////////////////////////////////////////////////
map<BinaryData, shared_ptr<TxIOPair>> ZeroConfContainer::getUnspentZCforScrAddr(
   BinaryData scrAddr) const
{
   auto ss = getSnapshot();
   if (ss == nullptr)
      return map<BinaryData, shared_ptr<TxIOPair>>();

   auto& txiomapptr = ss->txioMap_;
   auto saIter = txiomapptr.find(scrAddr);

   if (saIter != txiomapptr.end())
   {
      auto& zcMap = saIter->second;
      map<BinaryData, shared_ptr<TxIOPair>> returnMap;

      for (auto& zcPair : zcMap)
      {
         if (zcPair.second->hasTxIn())
            continue;

         returnMap.insert(zcPair);
      }

      return returnMap;
   }

   return {};
}

///////////////////////////////////////////////////////////////////////////////
map<BinaryData, shared_ptr<TxIOPair>> ZeroConfContainer::getRBFTxIOsforScrAddr(
   BinaryData scrAddr) const
{
   auto ss = getSnapshot();
   auto& txiomapptr = ss->txioMap_;
   auto saIter = txiomapptr.find(scrAddr);

   if (saIter != txiomapptr.end())
   {
      auto& zcMap = saIter->second;
      map<BinaryData, shared_ptr<TxIOPair>> returnMap;

      for (auto& zcPair : zcMap)
      {
         if (!zcPair.second->hasTxIn())
            continue;

         if (!zcPair.second->isRBF())
            continue;

         returnMap.insert(zcPair);
      }

      return returnMap;
   }

   return map<BinaryData, shared_ptr<TxIOPair>>();
}

///////////////////////////////////////////////////////////////////////////////
vector<TxOut> ZeroConfContainer::getZcTxOutsForKey(
   const set<BinaryData>& keys) const
{
   vector<TxOut> result;
   auto ss = getSnapshot();
   auto& txmap = ss->txMap_;

   for (auto& key : keys)
   {
      auto zcKey = key.getSliceRef(0, 6);

      auto txIter = txmap.find(zcKey);
      if (txIter == txmap.end())
         continue;

      auto& theTx = *txIter->second;

      auto outIdRef = key.getSliceRef(6, 2);
      auto outId = READ_UINT16_BE(outIdRef);

      auto&& txout = theTx.tx_.getTxOutCopy(outId);
      result.push_back(move(txout));
   }

   return result;
}

///////////////////////////////////////////////////////////////////////////////
vector<UnspentTxOut> ZeroConfContainer::getZcUTXOsForKey(
   const set<BinaryData>& keys) const
{
   vector<UnspentTxOut> result;
   auto ss = getSnapshot();
   auto& txmap = ss->txMap_;

   for (auto& key : keys)
   {
      auto zcKey = key.getSliceRef(0, 6);

      auto txIter = txmap.find(zcKey);
      if (txIter == txmap.end())
         continue;

      auto& theTx = *txIter->second;

      auto outIdRef = key.getSliceRef(6, 2);
      auto outId = READ_UINT16_BE(outIdRef);

      auto&& txout = theTx.tx_.getTxOutCopy(outId);
      UnspentTxOut utxo(
         theTx.getTxHash(), outId, UINT32_MAX,
         txout.getValue(), txout.getScript());

      result.push_back(move(utxo));
   }

   return result;
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::updateZCinDB()
{
   while (true)
   {
      ZcUpdateBatch batch;
      try
      {
         batch = move(updateBatch_.pop_front());
      }
      catch (StopBlockingLoop&)
      {
         break;
      }

      if (!batch.hasData())
         continue;

      auto&& tx = db_->beginTransaction(ZERO_CONF, LMDB::ReadWrite);
      for (auto& zc_pair : batch.zcToWrite_)
      {
            /*TODO: speed this up*/
            StoredTx zcTx;
            zcTx.createFromTx(zc_pair.second->tx_, true, true);
            db_->putStoredZC(zcTx, zc_pair.first);
      }

      for (auto& txhash : batch.txHashes_)
      {
         //if the key is not to be found in the txMap_, this is a ZC txhash
         db_->putValue(ZERO_CONF, txhash, BinaryData());
      }

      for (auto& key : batch.keysToDelete_)
      {
         BinaryData keyWithPrefix;
         if (key.getSize() == 6)
         {
            keyWithPrefix.resize(7);
            uint8_t* keyptr = keyWithPrefix.getPtr();
            keyptr[0] = DB_PREFIX_ZCDATA;
            memcpy(keyptr + 1, key.getPtr(), 6);
         }
         else
         {
            keyWithPrefix = key;
         }

         auto dbIter = db_->getIterator(ZERO_CONF);

         if (!dbIter->seekTo(keyWithPrefix))
            continue;

         vector<BinaryData> ktd;
         ktd.push_back(keyWithPrefix);

         do
         {
            BinaryDataRef thisKey = dbIter->getKeyRef();
            if (!thisKey.startsWith(keyWithPrefix))
               break;

            ktd.push_back(thisKey);
         } while (dbIter->advanceAndRead(DB_PREFIX_ZCDATA));

         for (auto _key : ktd)
            db_->deleteValue(ZERO_CONF, _key);
      }

      for (auto& key : batch.txHashesToDelete_)
         db_->deleteValue(ZERO_CONF, key);

      batch.setCompleted(true);
   }
}

///////////////////////////////////////////////////////////////////////////////
unsigned ZeroConfContainer::loadZeroConfMempool(bool clearMempool)
{
   unsigned topId = 0;
   map<BinaryDataRef, shared_ptr<ParsedTx>> zcMap;

   {
      auto&& tx = db_->beginTransaction(ZERO_CONF, LMDB::ReadOnly);
      auto dbIter = db_->getIterator(ZERO_CONF);

      if (!dbIter->seekToStartsWith(DB_PREFIX_ZCDATA))
         return topId;

      do
      {
         BinaryDataRef zcKey = dbIter->getKeyRef();

         if (zcKey.getSize() == 7)
         {
            //Tx, grab it from DB
            StoredTx zcStx;
            db_->getStoredZcTx(zcStx, zcKey);

            //add to newZCMap_
            auto&& zckey = zcKey.getSliceCopy(1, 6);
            Tx zctx(zcStx.getSerializedTx());
            zctx.setTxTime(zcStx.unixTime_);

            auto parsedTx = make_shared<ParsedTx>(zckey);
            parsedTx->tx_ = move(zctx);

            zcMap.insert(move(make_pair(
               parsedTx->getKeyRef(), move(parsedTx))));
         }
         else if (zcKey.getSize() == 9)
         {
            //TxOut, ignore it
            continue;
         }
         else if (zcKey.getSize() == 32)
         {
            //tx hash
            allZcTxHashes_.insert(zcKey);
         }
         else
         {
            //shouldn't hit this
            LOGERR << "Unknown key found in ZC mempool";
            break;
         }
      } while (dbIter->advanceAndRead(DB_PREFIX_ZCDATA));
   }

   if (clearMempool == true)
   {
      LOGWARN << "Mempool was flagged for deletion!";
      ZcUpdateBatch batch;
      auto fut = batch.getCompletedFuture();

      for (const auto& zcTx : zcMap)
         batch.keysToDelete_.insert(zcTx.first);

      updateBatch_.push_back(move(batch));
      fut.wait();
   }
   else if (zcMap.size())
   {
      preprocessZcMap(zcMap);

      //set highest used index
      auto lastEntry = zcMap.rbegin();
      auto& topZcKey = lastEntry->first;
      topId = READ_UINT32_BE(topZcKey.getSliceCopy(2, 4)) + 1;

      //no need to update the db nor notify bdvs on init
      map<BinaryData, WatcherTxBody> emptyWatcherMap;
      parseNewZC(
         move(zcMap), nullptr, false, false, 
         make_pair(string(), string()), 
         emptyWatcherMap);
   }

   return topId;
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::init(shared_ptr<ScrAddrFilter> saf, bool clearMempool)
{
   LOGINFO << "Enabling zero-conf tracking";

   scrAddrMap_ = saf->getZcFilterMapPtr();
   auto topId = loadZeroConfMempool(clearMempool);

   auto newZcPacketLbd = [this](ZcActionStruct zas)->void
   {
      this->parseNewZC(move(zas));
   };
   actionQueue_ = make_unique<ZcActionQueue>(
      newZcPacketLbd, zcPreprocessQueue_, topId);

   auto updateZcThread = [this](void)->void
   {
      updateZCinDB();
   };

   auto invTxThread = [this](void)->void
   {
      handleInvTx();
   };

   parserThreads_.push_back(thread(updateZcThread));
   parserThreads_.push_back(thread(invTxThread));
   increaseParserThreadPool(1);

   zcEnabled_.store(true, memory_order_relaxed);
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::pushZcPreprocessVec(shared_ptr<RequestZcPacket> req)
{
   if (req->hashes_.size() == 0)
      return;

   //register batch with main zc processing thread
   actionQueue_->initiateZcBatch(
      req->hashes_, ZC_GETDATA_TIMEOUT_MS, {}, false);

   //queue up individual requests for parser threads to process
   zcPreprocessQueue_->push_back(move(req));
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::handleInvTx()
{
   while (true)
   {
      shared_ptr<ZcPreprocessPacket> packet;
      try
      {
         packet = move(zcWatcherQueue_.pop_front());
      }
      catch(StopBlockingLoop&)
      {
         break;
      }

      switch (packet->type())
      {
      case ZcPreprocessPacketType_Inv:
      {
         //skip this entirely if there are no addresses to scan the ZCs against
         if (scrAddrMap_->size() == 0 &&
            BlockDataManagerConfig::getDbType() != ARMORY_DB_SUPER)
            continue;

         auto invPayload = dynamic_pointer_cast<ZcInvPayload>(packet);
         if (invPayload == nullptr)
            throw runtime_error("packet type mismatch");

         if (invPayload->watcher_)
         {
            /*
            This is an inv tx payload from the watcher node, check it against 
            our outstanding broadcasts
            */
            SingleLock lock(&watcherMapMutex_);
            for (auto& invEntry : invPayload->invVec_)
            {
               BinaryData bd(invEntry.hash, sizeof(invEntry.hash));
               auto iter = watcherMap_.find(bd);
               if (iter == watcherMap_.end() || 
                  iter->second.inved_ ||
                  iter->second.ignoreWatcherNodeInv_)
               {
                  continue;
               }

               //mark as fetched
               iter->second.inved_ = true;

               //set parsedTx tx body
               auto payloadTx = make_shared<ProcessPayloadTxPacket>(bd);
               payloadTx->rawTx_ = iter->second.rawTxPtr_;

               //push to preprocess threads
               actionQueue_->queueGetDataResponse(move(payloadTx));
            }
         }
         else
         {         
            /*
            inv tx from the process node, send a getdata request for these hashes
            */

            auto& invVec = invPayload->invVec_;
            if (parserThreadCount_ < invVec.size() &&
               parserThreadCount_ < maxZcThreadCount_)
               increaseParserThreadPool(invVec.size());

            auto request = make_shared<RequestZcPacket>();
            SingleLock lock(&watcherMapMutex_);

            for (auto& entry : invVec)
            {
               BinaryDataRef hash(entry.hash, sizeof(entry.hash));

               /*
               Skip this hash if it's in our watcher map. Invs from the network 
               will never trigger this condition. Invs from the tx we broadcast
               through the p2p interface neither, as we present the hash to 
               kickstart the chain of events (node won't inv back a hash it was
               inv'ed to).

               Only a native RPC broadcast can trigger this condition, as the
               node will inv all peers it has not received this hash from. We do
               not want to create an unnecessary batch for native RPC pushes, so
               we skip those.
               */
               if (watcherMap_.find(hash) != watcherMap_.end())
                  continue;

               request->hashes_.emplace_back(hash);
            }
         
            if (request->hashes_.empty())
               break;

            pushZcPreprocessVec(request);
         }

         //register batch with main zc processing thread
         break;
      }

      default: 
         throw runtime_error("invalid packet");
      }
   }
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::handleZcProcessingStructThread(void)
{
   while (1)
   {
      shared_ptr<ZcGetPacket> packet;
      try
      {
         packet = move(zcPreprocessQueue_->pop_front());
      }
      catch (StopBlockingLoop&)
      {
         break;
      }

      switch (packet->type_)
      {
      case ZcGetPacketType_Request:
      {
         auto request = dynamic_pointer_cast<RequestZcPacket>(packet);
         if (request != nullptr)
            requestTxFromNode(*request);

         break;
      }

      case ZcGetPacketType_Payload:
      {
         auto payloadTx = dynamic_pointer_cast<ProcessPayloadTxPacket>(packet);
         if (payloadTx == nullptr)
            throw runtime_error("unexpected payload type");

         processPayloadTx(payloadTx);
         break;
      }

      case ZcGetPacketType_Broadcast:
      {
         auto broadcastPacket = dynamic_pointer_cast<ZcBroadcastPacket>(packet);
         if (broadcastPacket == nullptr)
            break;
            
         pushZcPacketThroughP2P(*broadcastPacket);
         break;
      }

      default:
         break;
      } //switch
   } //while
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::processTxGetDataReply(unique_ptr<Payload> payload)
{
   switch (payload->type())
   {
   case Payload_tx:
   {
      shared_ptr<Payload> payload_sptr(move(payload));
      auto payloadtx = dynamic_pointer_cast<Payload_Tx>(payload_sptr);
      if (payloadtx == nullptr || payloadtx->getSize() == 0)
      {
         LOGERR << "invalid tx getdata payload";
         return;
      }

      //got a tx, post it to the zc preprocessing queue
      auto txData = make_shared<ProcessPayloadTxPacket>(payloadtx->getHash256());
      txData->rawTx_ = make_shared<BinaryData>(
         &payloadtx->getRawTx()[0], payloadtx->getRawTx().size());

      actionQueue_->queueGetDataResponse(move(txData));
      break;
   }

   case Payload_reject:
   {
      shared_ptr<Payload> payload_sptr(move(payload));
      auto payloadReject = dynamic_pointer_cast<Payload_Reject>(payload_sptr);
      if (payloadReject == nullptr)
      {
         LOGERR << "invalid reject payload";
         return;
      }

      if (payloadReject->rejectType() != Payload_tx)
      {
         //only handling payload_tx rejections
         return;
      }

      BinaryData hash(
         &payloadReject->getExtra()[0], 
         payloadReject->getExtra().size());

      auto rejectPacket = make_shared<RejectPacket>(hash, payloadReject->code());
      actionQueue_->queueGetDataResponse(move(rejectPacket));
      break;
   }

   default:
      break;
   }
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::requestTxFromNode(RequestZcPacket& packet)
{
   vector<InvEntry> invVec;
   for (auto& hash : packet.hashes_)
   {
      if (hash.getSize() != 32)
         throw runtime_error("invalid inv hash length");

      InvEntry inv;
      inv.invtype_ = Inv_Msg_Witness_Tx;
      memcpy(inv.hash, hash.getPtr(), 32);
      invVec.emplace_back(move(inv));
   }
   networkNode_->requestTx(move(invVec));
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::processPayloadTx(
   shared_ptr<ProcessPayloadTxPacket> payloadPtr)
{
   if (payloadPtr->rawTx_->getSize() == 0)
   {
      payloadPtr->pTx_->state_ = ParsedTxStatus::Tx_Invalid;
      payloadPtr->incrementCounter();
      return;
   }

   //set raw tx and current time
   payloadPtr->pTx_->tx_.unserialize(*payloadPtr->rawTx_);
   payloadPtr->pTx_->tx_.setTxTime(time(0));

   preprocessTx(*payloadPtr->pTx_);
   payloadPtr->incrementCounter();
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::broadcastZC(
   const vector<BinaryDataRef>& rawZcVec, uint32_t timeout_ms,
   const ZcBroadcastCallback& cbk, 
   const string& bdvID, const string& requestID)
{
   auto zcPacket = make_shared<ZcBroadcastPacket>();
   zcPacket->hashes_.reserve(rawZcVec.size());
   zcPacket->zcVec_.reserve(rawZcVec.size());

   for (auto& rawZcRef : rawZcVec)
   {
      if (rawZcRef.getSize() == 0)
         continue;

      auto rawZcPtr = make_shared<BinaryData>(rawZcRef);
      Tx tx(*rawZcPtr);

      zcPacket->hashes_.push_back(tx.getThisHash());
      zcPacket->zcVec_.push_back(rawZcPtr);
   }

   if (zcPacket->zcVec_.empty())
      return;

   {
      //update the watcher map
      ReentrantLock lock(&watcherMapMutex_);
      for (unsigned i=0; i < zcPacket->hashes_.size(); i++)
      {
         auto& hash = zcPacket->hashes_[i];
         map<string, string> emptyMap;
         if (setWatcherEntry(
            hash, zcPacket->zcVec_[i],
            bdvID, requestID, emptyMap))
         {
            continue;
         }

         //already have this zc in an earlier batch, drop the hash
         hash.clear();
      }
   }

   //sets up & queues the zc batch for us
   if (actionQueue_->initiateZcBatch(
      zcPacket->hashes_, timeout_ms, cbk, true, bdvID, requestID) == nullptr)
   {
      //return if no batch was created
      return;
   }

   //push each zc on the process queue
   zcPreprocessQueue_->push_back(zcPacket);
}

///////////////////////////////////////////////////////////////////////////////
bool ZeroConfContainer::setWatcherEntry(
   const BinaryData& hash, shared_ptr<BinaryData> rawTxPtr, 
   const std::string& bdvID, const std::string& requestID,
   std::map<std::string, std::string>& extraRequestors,
   bool watchEntry)
{
   //lock 
   ReentrantLock lock(&watcherMapMutex_);

   //try to insert
   auto insertIter = watcherMap_.insert(make_pair(
      hash, WatcherTxBody(rawTxPtr)));

   if (insertIter.second)
   {
      //set the watcher node flag
      insertIter.first->second.ignoreWatcherNodeInv_ = !watchEntry;

      //set extra requestors
      if (!extraRequestors.empty())
         insertIter.first->second.extraRequestors_ = move(extraRequestors);

      //return successful insertion
      return true;
   }
   else
   {
      //already have this hash, do not change the watcher node flag

      //tie this request to the existing watcher entry
      insertIter.first->second.extraRequestors_.emplace(
         requestID, bdvID);

      //add the extra requestors if any
      if (!extraRequestors.empty())
      {
         insertIter.first->second.extraRequestors_.insert(
            extraRequestors.begin(), extraRequestors.end());
      }

      //return failed insertion
      return false;
   }
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::pushZcPacketThroughP2P(ZcBroadcastPacket& packet)
{
   if (!networkNode_->connected())
   {
      LOGWARN << "node is offline, cannot broadcast";

      //TODO: report node down errors to batch
      /*packet.errorCallback_(
         move(*packet.rawZc_), ZCBroadcastStatus_P2P_NodeDown);*/
      return;
   }

   //create inv payload
   vector<InvEntry> invVec;
   map<BinaryData, shared_ptr<BitcoinP2P::getDataPayload>> getDataPair;

   for (unsigned i=0; i<packet.hashes_.size(); i++)
   {
      auto& hash = packet.hashes_[i];
      if (hash.empty())
         continue;
         
      auto& rawZc = packet.zcVec_[i];

      //create inv entry, this announces the zc by its hash to the node
      InvEntry entry;
      entry.invtype_ = Inv_Msg_Witness_Tx;
      memcpy(entry.hash, hash.getPtr(), 32);
      invVec.push_back(entry);

      //create getData payload packet, this is the zc body for the node to
      //grab once it knows of the hash
      auto&& payload = make_unique<Payload_Tx>();
      vector<uint8_t> rawtx;
      rawtx.resize(rawZc->getSize());
      memcpy(&rawtx[0], rawZc->getPtr(), rawZc->getSize());

      payload->setRawTx(move(rawtx));
      auto getDataPayload = make_shared<BitcoinP2P::getDataPayload>();
      getDataPayload->payload_ = move(payload);

      getDataPair.emplace(hash, getDataPayload);
   }

   //register getData payload
   networkNode_->getDataPayloadMap_.update(move(getDataPair));

   //send inv packet
   auto payload_inv = make_unique<Payload_Inv>();
   payload_inv->setInvVector(invVec);
   networkNode_->sendMessage(move(payload_inv));
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::shutdown()
{
   if (actionQueue_ != nullptr)
      actionQueue_->shutdown();

   zcWatcherQueue_.terminate();
   zcPreprocessQueue_->terminate();
   updateBatch_.terminate();

   vector<thread::id> idVec;
   for (auto& parser : parserThreads_)
   {
      idVec.push_back(parser.get_id());
      if (parser.joinable())
         parser.join();
   }
}

///////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::increaseParserThreadPool(unsigned count)
{
   unique_lock<mutex> lock(parserThreadMutex_);

   //start Zc parser thread
   auto processZcThread = [this](void)->void
   {
      handleZcProcessingStructThread();
   };

   for (unsigned i = parserThreadCount_; i < count; i++)
      parserThreads_.push_back(thread(processZcThread));

   parserThreadCount_ = parserThreads_.size();
   LOGINFO << "now running " << parserThreadCount_ << " zc parser threads";
}

///////////////////////////////////////////////////////////////////////////////
const map<BinaryData, shared_ptr<TxIOPair>>&
   ZeroConfContainer::getTxioMapForScrAddr(const BinaryData& scrAddr) const
{
   auto ss = getSnapshot();
   auto& txiomap = ss->txioMap_;

   auto iter = txiomap.find(scrAddr);
   if (iter == txiomap.end())
      throw runtime_error("no txio for this scraddr");

   return iter->second;
}

///////////////////////////////////////////////////////////////////////////////
shared_ptr<ParsedTx> ZeroConfContainer::getTxByKey(const BinaryData& key) const
{
   auto ss = getSnapshot();
   auto iter = ss->txMap_.find(key.getRef());
   if (iter == ss->txMap_.end())
      return nullptr;

   return iter->second;
}

///////////////////////////////////////////////////////////////////////////////
BinaryDataRef ZeroConfContainer::getKeyForHash(const BinaryDataRef& hash) const
{
   auto ss = getSnapshot();
   auto iter = ss->txHashToDBKey_.find(hash);
   if (iter == ss->txHashToDBKey_.end())
      return BinaryDataRef();

   return iter->second;
}

///////////////////////////////////////////////////////////////////////////////
BinaryDataRef ZeroConfContainer::getHashForKey(const BinaryDataRef& key) const
{
   auto ss = getSnapshot();
   auto iter = ss->txMap_.find(key);
   if (iter == ss->txMap_.end())
      return BinaryDataRef();

   return iter->second->getTxHash().getRef();
}

///////////////////////////////////////////////////////////////////////////////
TxOut ZeroConfContainer::getTxOutCopy(
   const BinaryDataRef key, unsigned index) const
{
   auto&& tx = getTxByKey(key);
   if (tx == nullptr)
      return TxOut();

   return tx->tx_.getTxOutCopy(index);
}

////////////////////////////////////////////////////////////////////////////////
void ZeroConfContainer::setWatcherNode(
   shared_ptr<BitcoinNodeInterface> watcherNode)
{
   auto getTxLambda = [this](vector<InvEntry> invVec)->void
   {
      if (!zcEnabled_.load(std::memory_order_relaxed))
         return;

      //push inv vector as watcher inv packet on the preprocessing queue
      auto payload = make_shared<ZcInvPayload>(true);
      payload->invVec_ = move(invVec);
      zcWatcherQueue_.push_back(move(payload));
   };

   watcherNode->registerInvTxLambda(getTxLambda);
}

////////////////////////////////////////////////////////////////////////////////
BatchTxMap ZeroConfContainer::getBatchTxMap(shared_ptr<ZeroConfBatch> batch, 
   shared_ptr<ZeroConfSharedStateSnapshot> ss)
{
   if (batch == nullptr)
      throw ZcBatchError();

   //wait on the batch for the duration of the 
   //timeout minus time elapsed since creation
   unsigned timeLeft = 0;
   auto delay = chrono::duration_cast<chrono::milliseconds>(
      chrono::system_clock::now() - batch->creationTime_);
   if (delay.count() < batch->timeout_)
      timeLeft = batch->timeout_ - delay.count();

   auto timeLeftMs = chrono::milliseconds(timeLeft);

   ArmoryErrorCodes batchResult;
   if (batch->timeout_ > 0 && 
      batch->isReadyFut_.wait_for(timeLeftMs) != future_status::ready)
   {
      batchResult = ArmoryErrorCodes::ZcBatch_Timeout;
   }
   else
   {
      batchResult = batch->isReadyFut_.get();
   }

   BatchTxMap result;
   result.requestor_ = batch->requestor_;

   //purge the watcher map if this batch affects it
   if (batch->hasWatcherEntries_)
   {
      SingleLock lock(&watcherMapMutex_);
      for (auto& keyPair : batch->hashToKeyMap_)
      {
         auto iter = watcherMap_.find(keyPair.first);
         if (iter == watcherMap_.end())
         {
            LOGERR << "missing watcher entry, this should not happen!";
            LOGERR << "skipping this timed out batch, this needs reported to a dev";
            throw ZcBatchError();
         }

         result.watcherMap_.emplace(iter->first, move(iter->second));
         watcherMap_.erase(iter);
      }
   }

   if (batchResult != ArmoryErrorCodes::Success)
   {
      /*
      Failed to get transactions for batch, fire the error callback
      */
      
      //skip if this batch doesn't have a callback
      if (!batch->errorCallback_ || !batch->hasWatcherEntries_)
         throw ZcBatchError();

      unsigned invedZcCount = 0;
      vector<ZeroConfBatchFallbackStruct> txVec;
      set<BinaryDataRef> purgedHashes;
      txVec.reserve(batch->zcMap_.size());

      //purge the batch of missing tx and their children
      for (auto& txPair : batch->zcMap_)
      {
         bool purge = false;

         //does this tx depend on a purged predecessor?
         for (auto& txInObj : txPair.second->inputs_)
         {
            auto parentIter = purgedHashes.find(
               txInObj.opRef_.getTxHashRef());

            if (parentIter == purgedHashes.end())
               continue;

            //this zc depends on a purged zc, purge it too
            purge = true;
            break;
         }

         //was this tx inv'ed back to us?
         bool inved = true;
         auto iter = result.watcherMap_.find(txPair.second->getTxHash());

         //map consistency is assured in the watcherMap purge 
         //scope, this iterator is guaranteed valid
         if (!iter->second.inved_)
         {
            inved = false;
            purge = true;
         }
                        
         if (!purge)
         {
            //sanity check
            if (!inved)
            {
               LOGWARN << "keeping zc from timed out batch"
                  << " that wasn't inved";
            }

            //we're keeping this tx
            ++invedZcCount;
            continue;
         }

         //create the fallback struct
         ZeroConfBatchFallbackStruct fallbackStruct;
         fallbackStruct.txHash_ = iter->first;
         fallbackStruct.rawTxPtr_ = move(iter->second.rawTxPtr_);
         fallbackStruct.err_ = batchResult;
         fallbackStruct.extraRequestors_ = move(iter->second.extraRequestors_);

         //check snapshot for collisions
         auto collisionIter = ss->txHashToDBKey_.find(iter->first.getRef());
         if (collisionIter != ss->txHashToDBKey_.end())
         {
            //already have this tx in our mempool, report to callback
            //but don't flag hash as purged (children need to be processed if any)
            fallbackStruct.err_ = ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool;
         }
         else
         {
            //keep track of purged zc hashes
            purgedHashes.insert(iter->first.getRef());
         }

         //flag tx to be skipped by parser
         txPair.second->state_ = ParsedTxStatus::Tx_Skip;

         //add to vector for error callback
         txVec.emplace_back(move(fallbackStruct));
      }

      batch->errorCallback_(move(txVec));

      //don't forward the batch if it has no zc ready to be parsed
      if (invedZcCount == 0)
         throw ZcBatchError();

      //we have some inv'ed zc to parse but the batch timed out, we need to
      //wait on the counter to match our local count of valid tx.
      while (batch->zcMap_.size() - batch->counter_->load(memory_order_acquire) < 
         invedZcCount)
      {
         LOGWARN << "timedout batch waiting on " << invedZcCount << " inved tx: ";
         LOGWARN << "batch size: " << batch->zcMap_.size() << ", counter: " << 
            batch->counter_->load(memory_order_acquire);
         this_thread::sleep_for(chrono::milliseconds(100));
      }
   }

   result.txMap_ = batch->zcMap_;
   return result;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//// ZcActionQueue
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void ZcActionQueue::start()
{
   auto processZcThread = [this](void)->void
   {
      processNewZcQueue();
   };

   auto matcherThread = [this](void)->void
   {
      getDataToBatchMatcherThread();
   };

   processThreads_.push_back(thread(processZcThread));
   processThreads_.push_back(thread(matcherThread));
}

////////////////////////////////////////////////////////////////////////////////
void ZcActionQueue::shutdown()
{
   newZcQueue_.terminate();
   getDataResponseQueue_.terminate();
   for (auto& thr : processThreads_)
   {
      if (thr.joinable())
         thr.join();
   }
}

////////////////////////////////////////////////////////////////////////////////
BinaryData ZcActionQueue::getNewZCkey()
{
   uint32_t newId = topId_.fetch_add(1, memory_order_relaxed);
   BinaryData newKey = READHEX("ffff");
   newKey.append(WRITE_UINT32_BE(newId));

   return move(newKey);
}

////////////////////////////////////////////////////////////////////////////////
shared_ptr<ZeroConfBatch> ZcActionQueue::initiateZcBatch(
   const vector<BinaryData>& zcHashes, unsigned timeout, 
   const ZcBroadcastCallback& cbk, bool hasWatcherEntries,
   const std::string& bdvId, const std::string& requestId)
{
   auto batch = make_shared<ZeroConfBatch>(hasWatcherEntries);
   batch->requestor_ = make_pair(requestId, bdvId);

   for (auto& hash : zcHashes)
   {
      //skip if hash is empty
      if (hash.empty())
         continue;

      auto&& key = getNewZCkey();
      auto ptx = make_shared<ParsedTx>(key);
      ptx->setTxHash(hash);

      batch->hashToKeyMap_.emplace(
         make_pair(ptx->getTxHash().getRef(), ptx->getKeyRef()));
      batch->zcMap_.emplace(make_pair(ptx->getKeyRef(), ptx));
   }

   if (batch->zcMap_.empty())
   {
      //empty batch, skip
      return nullptr;
   }

   batch->counter_->store(batch->zcMap_.size(), memory_order_relaxed);
   batch->timeout_ = timeout; //in milliseconds
   batch->errorCallback_ = cbk;

   ZcActionStruct zac;
   zac.action_ = Zc_NewTx;
   zac.batch_ = batch;
   newZcQueue_.push_back(move(zac));

   auto batchCopy = batch;
   batchQueue_.push_back(move(batchCopy));

   return batch;
}

////////////////////////////////////////////////////////////////////////////////
void ZcActionQueue::processNewZcQueue()
{
   while (1)
   {
      ZcActionStruct zcAction;
      map<BinaryDataRef, shared_ptr<ParsedTx>> zcMap;
      try
      {
         zcAction = move(newZcQueue_.pop_front());
      }
      catch (StopBlockingLoop&)
      {
         break;
      }

      /*
      Populate local map with batch's zcMap_ so that we can cleanup the
      hashes from the request map after parsing.
      */
      if (zcAction.batch_ != nullptr)
      {
         /*
         We can't just grab the hash reference since the object referred to is
         held by a ParsedTx and that has no guarantee of surviving the parsing 
         function, hence copying the entire map.
         */
         zcMap = zcAction.batch_->zcMap_;
      }

      newZcFunction_(move(zcAction));

      if (zcMap.empty())
         continue;

      //cleanup request map
      set<BinaryData> hashSet;
      for (auto& zcPair : zcMap)
         hashSet.insert(zcPair.second->getTxHash());
      hashesToClear_.push_back(move(hashSet));
   }
}

////////////////////////////////////////////////////////////////////////////////
shared_future<shared_ptr<ZcPurgePacket>> 
ZcActionQueue::pushNewBlockNotification(
   Blockchain::ReorganizationState reorgState)
{
   ZcActionStruct zcaction;
   zcaction.action_ = Zc_Purge;
   zcaction.resultPromise_ =
      make_unique<promise<shared_ptr<ZcPurgePacket>>>();
   zcaction.reorgState_ = reorgState;

   auto fut = zcaction.resultPromise_->get_future();
   newZcQueue_.push_back(move(zcaction));
   
   return fut;
}

////////////////////////////////////////////////////////////////////////////////
void ZcActionQueue::queueGetDataResponse(std::shared_ptr<ZcGetPacket> payloadTx)
{
   getDataResponseQueue_.push_back(move(payloadTx));
}

////////////////////////////////////////////////////////////////////////////////
void ZcActionQueue::getDataToBatchMatcherThread()
{
   bool run = true;
   map<BinaryData, shared_ptr<ZeroConfBatch>> hashToBatchMap;
   while (run)
   {
      //queue of outstanding node getdata packets that need matched with 
      //their parent batch - blocking
      shared_ptr<ZcGetPacket> zcPacket;
      try
      {    
         zcPacket = getDataResponseQueue_.pop_front();
      }
      catch (const StopBlockingLoop&)
      {
         run = false;
      }

      //queue of new batches - non blocking
      while (true)
      {
         try
         {
            auto batch = batchQueue_.pop_front();

            //populate local map with hashes from this batch, do not
            //overwrite existing entries (older batches should get 
            //precedence over a shared tx hash)
            for (auto& hashPair : batch->hashToKeyMap_)
               hashToBatchMap.emplace(hashPair.first, batch);
         }
         catch (const IsEmpty&)
         {
            break;
         }
      }

      if (zcPacket != nullptr)
      {
         switch (zcPacket->type_)
         {
         case ZcGetPacketType_Payload:
         {
            auto payloadTx = dynamic_pointer_cast<ProcessPayloadTxPacket>(zcPacket);
            if (payloadTx == nullptr)
               break;

            //look for parent batch in local map
            auto iter = hashToBatchMap.find(payloadTx->txHash_);
            if (iter != hashToBatchMap.end())
            {
               //tie the tx to its batch
               payloadTx->batchCtr_ = iter->second->counter_;
               payloadTx->batchProm_ = iter->second->isReadyPromise_;
                  
               auto keyIter = iter->second->hashToKeyMap_.find(
                  payloadTx->txHash_.getRef());
               if (keyIter != iter->second->hashToKeyMap_.end())
               { 
                  auto txIter = iter->second->zcMap_.find(keyIter->second);
                  if (txIter != iter->second->zcMap_.end())
                  {
                     payloadTx->pTx_ = txIter->second;
                     zcPreprocessQueue_->push_back(payloadTx);
                  }
               }

               hashToBatchMap.erase(iter);
            }

            break;
         }

         case ZcGetPacketType_Reject:
         {
            auto rejectPacket = dynamic_pointer_cast<RejectPacket>(zcPacket);
            if (rejectPacket == nullptr)
               break;

            //grab the batch
            auto iter = hashToBatchMap.find(rejectPacket->txHash_);
            if (iter != hashToBatchMap.end())
            {
               iter->second->isReadyPromise_->set_value(
                  ArmoryErrorCodes(rejectPacket->code_));
               
               hashToBatchMap.erase(iter);
            }

            break;
         }

         default:
            break;
         }
      }

      //queue of hashes to purge from the local map
      while (true)
      {
         try
         {
            auto&& hashSet = hashesToClear_.pop_front();
            for (auto& hash : hashSet)
               hashToBatchMap.erase(hash);
         }
         catch (const IsEmpty&)
         {
            break;
         }
      }

      matcherMapSize_.store(hashToBatchMap.size(), memory_order_relaxed);
   }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//// OutPointRef
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void OutPointRef::unserialize(uint8_t const * ptr, uint32_t remaining)
{
   if (remaining < 36)
      throw runtime_error("ptr is too short to be an outpoint");

   BinaryDataRef bdr(ptr, remaining);
   BinaryRefReader brr(bdr);

   txHash_ = brr.get_BinaryDataRef(32);
   txOutIndex_ = brr.get_uint32_t();
}

////////////////////////////////////////////////////////////////////////////////
void OutPointRef::unserialize(BinaryDataRef bdr)
{
   unserialize(bdr.getPtr(), bdr.getSize());
}

////////////////////////////////////////////////////////////////////////////////
void OutPointRef::resolveDbKey(LMDBBlockDatabase *dbPtr)
{
   if (txHash_.getSize() == 0 || txOutIndex_ == UINT16_MAX)
      throw runtime_error("empty outpoint hash");

   auto&& key = dbPtr->getDBKeyForHash(txHash_);
   if (key.getSize() != 6)
      return;

   BinaryWriter bw;
   bw.put_BinaryData(key);
   bw.put_uint16_t(txOutIndex_, BE);

   dbKey_ = bw.getData();
}

////////////////////////////////////////////////////////////////////////////////
BinaryDataRef OutPointRef::getDbTxKeyRef() const
{
   if (!isResolved())
      throw runtime_error("unresolved outpoint key");

   return dbKey_.getSliceRef(0, 6);
}

////////////////////////////////////////////////////////////////////////////////
bool OutPointRef::isInitialized() const
{
   return txHash_.getSize() == 32 && txOutIndex_ != UINT16_MAX;
}

////////////////////////////////////////////////////////////////////////////////
bool OutPointRef::isZc() const
{
   if (!isResolved())
      return false;

   auto ptr = dbKey_.getPtr();
   auto val = (uint16_t*)ptr;
   return *val == 0xFFFF;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//// ParsedTxIn
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool ParsedTxIn::isResolved() const
{
   if (!opRef_.isResolved())
      return false;

   if (scrAddr_.getSize() == 0 || value_ == UINT64_MAX)
      return false;

   return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//// ParsedTx
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
bool ParsedTx::isResolved() const
{
   if (state_ == Tx_Uninitialized)
      return false;

   if (!tx_.isInitialized())
      return false;

   if (inputs_.size() != tx_.getNumTxIn() ||
      outputs_.size() != tx_.getNumTxOut())
      return false;

   for (auto& input : inputs_)
   {
      if (!input.isResolved())
         return false;
   }

   return true;
}

////////////////////////////////////////////////////////////////////////////////
void ParsedTx::reset()
{
   for (auto& input : inputs_)
      input.opRef_.reset();
   tx_.setChainedZC(false);

   state_ = Tx_Uninitialized;
}

////////////////////////////////////////////////////////////////////////////////
const BinaryData& ParsedTx::getTxHash(void) const
{
   if (txHash_.getSize() == 0)
      txHash_ = move(tx_.getThisHash());
   return txHash_;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//// ZeroConfCallbacks
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
ZeroConfCallbacks::~ZeroConfCallbacks() 
{}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//// ZcUpdateBatch
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
shared_future<bool> ZcUpdateBatch::getCompletedFuture()
{
   if (completed_ == nullptr)
      completed_ = make_unique<promise<bool>>();
   return completed_->get_future();
}

////////////////////////////////////////////////////////////////////////////////
void ZcUpdateBatch::setCompleted(bool val)
{
   if (completed_ == nullptr)
      return;

   completed_->set_value(val);
}

////////////////////////////////////////////////////////////////////////////////
bool ZcUpdateBatch::hasData() const
{
   if (zcToWrite_.size() > 0 ||
      txHashes_.size() > 0 ||
      keysToDelete_.size() > 0)
      return true;
   
   return false;
}