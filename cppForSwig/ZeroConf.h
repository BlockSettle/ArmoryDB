////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Copyright (C) 2011-2015, Armory Technologies, Inc.                        //
//  Distributed under the GNU Affero General Public License (AGPL v3)         //
//  See LICENSE-ATI or http://www.gnu.org/licenses/agpl.html                  //
//                                                                            //
//                                                                            //
//  Copyright (C) 2016-2020, goatpig                                          //            
//  Distributed under the MIT license                                         //
//  See LICENSE-MIT or https://opensource.org/licenses/MIT                    //                                   
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _ZEROCONF_H_
#define _ZEROCONF_H_

#include <vector>
#include <atomic>
#include <functional>
#include <memory>

#include "ThreadSafeClasses.h"
#include "BitcoinP2p.h"
#include "lmdb_wrapper.h"
#include "Blockchain.h"
#include "ScrAddrFilter.h"
#include "ArmoryErrors.h"
#include "ZeroConfNotifications.h"

#define GETZC_THREADCOUNT 5

enum ZcAction
{
   Zc_NewTx,
   Zc_Purge,
   Zc_Shutdown
};

enum ParsedTxStatus
{
   Tx_Uninitialized,
   Tx_Resolved,
   Tx_ResolveAgain,
   Tx_Unresolved,
   Tx_Mined,
   Tx_Invalid,
   Tx_Skip
};

////////////////////////////////////////////////////////////////////////////////
struct ZeroConfData
{
   Tx            txobj_;
   uint32_t      txtime_;

   bool operator==(const ZeroConfData& rhs) const
   {
      return (this->txobj_ == rhs.txobj_) && (this->txtime_ == rhs.txtime_);
   }
};

////////////////////////////////////////////////////////////////////////////////
class OutPointRef
{
private:
   BinaryData txHash_;
   unsigned txOutIndex_ = UINT16_MAX;
   BinaryData dbKey_;
   uint64_t time_ = UINT64_MAX;

public:
   void unserialize(uint8_t const * ptr, uint32_t remaining);
   void unserialize(BinaryDataRef bdr);

   void resolveDbKey(LMDBBlockDatabase* db);
   const BinaryData& getDbKey(void) const { return dbKey_; }

   bool isResolved(void) const { return dbKey_.getSize() == 8; }
   bool isInitialized(void) const;

   BinaryDataRef getTxHashRef(void) const { return txHash_.getRef(); }
   unsigned getIndex(void) const { return txOutIndex_; }

   BinaryData& getDbKey(void) { return dbKey_; }
   BinaryDataRef getDbTxKeyRef(void) const;

   void reset(void)
   {
      dbKey_.clear();
      time_ = UINT64_MAX;
   }

   bool isZc(void) const;

   void setTime(uint64_t t) { time_ = t; }
   uint64_t getTime(void) const { return time_; }
};

////////////////////////////////////////////////////////////////////////////////
struct ParsedTxIn
{
   OutPointRef opRef_;
   BinaryData scrAddr_;
   uint64_t value_ = UINT64_MAX;

public:
   bool isResolved(void) const;
};

////////////////////////////////////////////////////////////////////////////////
struct ParsedTxOut
{
   BinaryData scrAddr_;
   uint64_t value_ = UINT64_MAX;

   size_t offset_;
   size_t len_;

   bool isInitialized(void) const
   {
      return scrAddr_.getSize() != 0 && value_ != UINT64_MAX; \
   }
};

////////////////////////////////////////////////////////////////////////////////
struct ParsedTx
{
private:
   mutable BinaryData txHash_;
   const BinaryData zcKey_;

public:
   Tx tx_;
   std::vector<ParsedTxIn> inputs_;
   std::vector<ParsedTxOut> outputs_;
   ParsedTxStatus state_ = Tx_Uninitialized;
   bool isRBF_ = false;
   bool isChainedZc_ = false;
   bool needsReparsed_ = false;

public:
   ParsedTx(BinaryData& key) :
      zcKey_(std::move(key))
   {
      //set zc index in Tx object
      BinaryRefReader brr(zcKey_.getRef());
      brr.advance(2);
      tx_.setTxIndex(brr.get_uint32_t(BE));
   }

   ParsedTxStatus status(void) const { return state_; }
   bool isResolved(void) const;
   void reset(void);

   const BinaryData& getTxHash(void) const;
   void setTxHash(const BinaryData& hash) { txHash_ = hash; }
   BinaryDataRef getKeyRef(void) const { return zcKey_.getRef(); }
   const BinaryData& getKey(void) const { return zcKey_; }
};

////////////////////////////////////////////////////////////////////////////////
struct ZeroConfBatchFallbackStruct
{
   BinaryData txHash_;
   std::shared_ptr<BinaryData> rawTxPtr_;
   std::map<std::string, std::string> extraRequestors_;

   ArmoryErrorCodes err_;
};

////////////////////////////////////////////////////////////////////////////////
typedef std::function<void(
   std::vector<ZeroConfBatchFallbackStruct>)> ZcBroadcastCallback;

struct ZcBatchError
{};

////////////////////////////////////////////////////////////////////////////////
struct ZeroConfBatch
{
   //<zcKey ref, ParsedTx>, ParsedTx carries the key object
   std::map<BinaryDataRef, std::shared_ptr<ParsedTx>> zcMap_;
   
   //<txHash ref, zcKey ref>, ParsedTx carries both hash and key objects
   std::map<BinaryDataRef, BinaryDataRef> hashToKeyMap_;

   std::shared_ptr<std::atomic<int>> counter_;
   std::shared_ptr<std::promise<ArmoryErrorCodes>> isReadyPromise_;
   std::shared_future<ArmoryErrorCodes> isReadyFut_;

   unsigned timeout_ = UINT32_MAX;
   std::chrono::system_clock::time_point creationTime_;
   ZcBroadcastCallback errorCallback_;

   const bool hasWatcherEntries_;

   //<request id, bdv id>
   std::pair<std::string, std::string> requestor_;

public:
   ZeroConfBatch(bool hasWatcherEntries) :
      hasWatcherEntries_(hasWatcherEntries)
   {
      counter_ = std::make_shared<std::atomic<int>>();
      isReadyPromise_ = std::make_shared<std::promise<ArmoryErrorCodes>>();
      isReadyFut_ = isReadyPromise_->get_future();
      creationTime_ = std::chrono::system_clock::now();
   }
};

////////////////////////////////////////////////////////////////////////////////
enum ZcPreprocessPacketType
{
   ZcPreprocessPacketType_Inv
};

////
class ZcPreprocessPacket
{
private:
   const ZcPreprocessPacketType type_;

public:
   ZcPreprocessPacket(ZcPreprocessPacketType type) :
      type_(type)
   {}

   virtual ~ZcPreprocessPacket(void) = 0;

   ZcPreprocessPacketType type(void) const { return type_; }
};

////
struct ZcInvPayload : public ZcPreprocessPacket
{
   const bool watcher_;

   ZcInvPayload(bool watcher) :
      ZcPreprocessPacket(ZcPreprocessPacketType_Inv), 
      watcher_(watcher)
   {}

   std::vector<InvEntry> invVec_;
};

////////////////////////////////////////////////////////////////////////////////
enum ZcGetPacketType
{
   ZcGetPacketType_Broadcast,
   ZcGetPacketType_Request,
   ZcGetPacketType_Payload,
   ZcGetPacketType_Reject
};

////
struct ZcGetPacket
{
   const ZcGetPacketType type_;

   ZcGetPacket(ZcGetPacketType type) :
      type_(type)
   {}

   virtual ~ZcGetPacket(void) = 0;
};

////
struct RequestZcPacket : public ZcGetPacket
{
   std::vector<BinaryData> hashes_;

   RequestZcPacket(void) : 
      ZcGetPacket(ZcGetPacketType_Request)
   {}
};

////
struct ProcessPayloadTxPacket : public ZcGetPacket
{
   std::shared_ptr<std::atomic<int>> batchCtr_;
   std::shared_ptr<std::promise<ArmoryErrorCodes>> batchProm_;

   const BinaryData txHash_;
   std::shared_ptr<BinaryData> rawTx_;
   std::shared_ptr<ParsedTx> pTx_;

   ProcessPayloadTxPacket(const BinaryData& hash) : 
      ZcGetPacket(ZcGetPacketType_Payload), txHash_(hash)
   {}

   void incrementCounter(void)
   {
      if (batchCtr_ == nullptr)
      {
         LOGERR << "null batch ptr";
         throw std::runtime_error("null batch ptr");
      }

      auto val = batchCtr_->fetch_sub(1, std::memory_order_release);
      if (val == 1)
      try
      {
         batchProm_->set_value(ArmoryErrorCodes::Success);
      }
      catch (const std::future_error&)
      {
         LOGWARN << "batch promise already set";
      }
   }
};

////
struct ZcBroadcastPacket : public ZcGetPacket
{
   std::vector<std::shared_ptr<BinaryData>> zcVec_;
   std::vector<BinaryData> hashes_;

   ZcBroadcastPacket(void) :
      ZcGetPacket(ZcGetPacketType_Broadcast)
   {}
};

////
struct RejectPacket : public ZcGetPacket
{
   const BinaryData txHash_;
   char code_;

   RejectPacket(const BinaryData& hash, char code) :
      ZcGetPacket(ZcGetPacketType_Reject), 
      txHash_(hash), code_(code)
   {}
};

////////////////////////////////////////////////////////////////////////////////
struct ZcUpdateBatch
{
private:
   std::unique_ptr<std::promise<bool>> completed_;

public:
   std::map<BinaryDataRef, std::shared_ptr<ParsedTx>> zcToWrite_;
   std::set<BinaryData> txHashes_;
   std::set<BinaryData> keysToDelete_;
   std::set<BinaryData> txHashesToDelete_;

   std::shared_future<bool> getCompletedFuture(void);
   void setCompleted(bool);
   bool hasData(void) const;
};

////////////////////////////////////////////////////////////////////////////////
struct ZeroConfSharedStateSnapshot
{
   std::map<BinaryDataRef, BinaryDataRef> txHashToDBKey_; //<txHash, zcKey>
   std::map<BinaryDataRef, std::shared_ptr<ParsedTx>> txMap_; //<zcKey, zcTx>
   std::set<BinaryData> txOutsSpentByZC_; //<txOutDbKeys>

   //TODO: rethink this map, slow to purge
   //<scrAddr,  <dbKeyOfOutput, TxIOPair>> 
   std::map<BinaryData, 
      std::map<BinaryData, std::shared_ptr<TxIOPair>>> txioMap_;

   static std::shared_ptr<ZeroConfSharedStateSnapshot> copy(
      std::shared_ptr<ZeroConfSharedStateSnapshot> obj)
   {
      auto ss = std::make_shared<ZeroConfSharedStateSnapshot>();
      if (obj != nullptr)
      {
         ss->txHashToDBKey_ = obj->txHashToDBKey_;
         ss->txMap_ = obj->txMap_;
         ss->txOutsSpentByZC_ = obj->txOutsSpentByZC_;
         ss->txioMap_ = obj->txioMap_;
      }

      return ss;
   }
};

////////////////////////////////////////////////////////////////////////////////
struct BatchTxMap
{
   std::map<BinaryDataRef, std::shared_ptr<ParsedTx>> txMap_;
   std::map<BinaryData, WatcherTxBody> watcherMap_;
   
   //<request id, bdv id>
   std::pair<std::string, std::string> requestor_;
};

////////////////////////////////////////////////////////////////////////////////
struct ZcActionStruct
{
   ZcAction action_;
   std::shared_ptr<ZeroConfBatch> batch_;
   std::unique_ptr<std::promise<std::shared_ptr<ZcPurgePacket>>> 
      resultPromise_ = nullptr;
   Blockchain::ReorganizationState reorgState_;
};

typedef ArmoryThreading::BlockingQueue<std::shared_ptr<ZcGetPacket>> PreprocessQueue;

////////////////////////////////////////////////////////////////////////////////
class ZcActionQueue
{
private:
   //ready batches will be passed to this function
   std::function<void(ZcActionStruct)> newZcFunction_;

   //getData responses that have been matched to their batch will be posted 
   //to this queue
   std::shared_ptr<PreprocessQueue> zcPreprocessQueue_;

   //current top ZC id, incremented as new zc is pushed from the node/broadcasts
   std::atomic<uint32_t> topId_;

   std::vector<std::thread> processThreads_;

   //queue of batches served to newZcFunction_
   ArmoryThreading::BlockingQueue<ZcActionStruct> newZcQueue_;

   //queue of batches for the matcher thread to populate its local map of 
   //hashes to batches
   ArmoryThreading::Queue<std::shared_ptr<ZeroConfBatch>> batchQueue_;

   //queue of getData response from the node
   ArmoryThreading::BlockingQueue<
      std::shared_ptr<ZcGetPacket>> getDataResponseQueue_;

   //queue of hashes to clear from macther thread local map
   ArmoryThreading::Queue<std::set<BinaryData>> hashesToClear_;

   //tracks the size of the matcher thread local map, for unit test 
   //coverage purposes
   std::atomic<unsigned> matcherMapSize_;

private:
   void processNewZcQueue(void);
   BinaryData getNewZCkey(void);

   //matcher thread
   void getDataToBatchMatcherThread(void);

public:
   ZcActionQueue(
      std::function<void(ZcActionStruct)> func, 
      std::shared_ptr<PreprocessQueue> zcPreprocessQueue,      
      unsigned topId) :
      newZcFunction_(func), zcPreprocessQueue_(zcPreprocessQueue)
   {
      topId_.store(topId, std::memory_order_relaxed);
      matcherMapSize_.store(0, std::memory_order_relaxed);
      start();
   }

   void start(void);
   void shutdown(void);

   std::shared_ptr<ZeroConfBatch> initiateZcBatch(
      const std::vector<BinaryData>&, unsigned,
      const ZcBroadcastCallback&, bool,
      const std::string& = "", const std::string& = "");

   std::shared_future<std::shared_ptr<ZcPurgePacket>> pushNewBlockNotification(
      Blockchain::ReorganizationState);
   
   void queueGetDataResponse(std::shared_ptr<ZcGetPacket>);
   void queueBatch(std::shared_ptr<ZeroConfBatch>);

   unsigned getMatcherMapSize(void) const 
   { 
      return matcherMapSize_.load(std::memory_order_relaxed); 
   }
};

////////////////////////////////////////////////////////////////////////////////
class ZeroConfContainer
{
private:
   struct BulkFilterData
   {
      std::map<BinaryData, std::map<BinaryData, std::shared_ptr<TxIOPair>>> scrAddrTxioMap_;
      std::map<BinaryDataRef, std::map<unsigned, BinaryDataRef>> outPointsSpentByKey_;
      std::set<BinaryData> txOutsSpentByZC_;
      std::map<BinaryDataRef, std::shared_ptr<std::set<BinaryDataRef>>> keyToSpentScrAddr_;
      std::map<BinaryDataRef, std::set<BinaryDataRef>> keyToFundedScrAddr_;

      std::map<std::string, ParsedZCData> flaggedBDVs_;

      bool isEmpty(void) { return scrAddrTxioMap_.size() == 0; }
   };

private:
   std::shared_ptr<ZeroConfSharedStateSnapshot> snapshot_;
   
   //<txHash, map<opId, ZcKeys>>
   std::map<BinaryDataRef, 
      std::map<unsigned, BinaryDataRef>> outPointsSpentByKey_;
   std::set<BinaryData> minedTxHashes_;

   //<zcKey, set<ScrAddr>>
   std::map<BinaryDataRef, 
      std::shared_ptr<std::set<BinaryDataRef>>> keyToSpentScrAddr_;
   
   std::set<BinaryData> allZcTxHashes_;
   std::map<BinaryDataRef, std::set<BinaryDataRef>> keyToFundedScrAddr_;

   LMDBBlockDatabase* db_;
   std::shared_ptr<BitcoinNodeInterface> networkNode_;

   std::shared_ptr<PreprocessQueue> zcPreprocessQueue_;
   ArmoryThreading::BlockingQueue<
      std::shared_ptr<ZcPreprocessPacket>> zcWatcherQueue_;
   ArmoryThreading::BlockingQueue<
      ZcUpdateBatch> updateBatch_;

   std::mutex parserMutex_;
   std::mutex parserThreadMutex_;

   std::vector<std::thread> parserThreads_;
   std::atomic<bool> zcEnabled_;
   const unsigned maxZcThreadCount_;

   std::shared_ptr<ArmoryThreading::TransactionalMap<
      BinaryDataRef, std::shared_ptr<AddrAndHash>>> scrAddrMap_;

   unsigned parserThreadCount_ = 0;
   std::unique_ptr<ZeroConfCallbacks> bdvCallbacks_;
   std::unique_ptr<ZcActionQueue> actionQueue_;

   std::map<BinaryData, WatcherTxBody> watcherMap_;
   ArmoryMutex watcherMapMutex_;

private:
   BulkFilterData ZCisMineBulkFilter(ParsedTx & tx, const BinaryDataRef& ZCkey,
      std::function<bool(const BinaryData&, BinaryData&)> getzckeyfortxhash,
      std::function<const ParsedTx&(const BinaryData&)> getzctxbykey);

   void preprocessTx(ParsedTx&) const;

   unsigned loadZeroConfMempool(bool clearMempool);
   bool purge(
      const Blockchain::ReorganizationState&, 
      std::shared_ptr<ZeroConfSharedStateSnapshot>,
      std::map<BinaryData, BinaryData>&);
   void reset(void);

   void processTxGetDataReply(std::unique_ptr<Payload> payload);
   void handleZcProcessingStructThread(void);
   void requestTxFromNode(RequestZcPacket&);
   void processPayloadTx(std::shared_ptr<ProcessPayloadTxPacket>);

   void increaseParserThreadPool(unsigned);
   void preprocessZcMap(std::map<BinaryDataRef, std::shared_ptr<ParsedTx>>&);

   void pushZcPacketThroughP2P(ZcBroadcastPacket&);
   void pushZcPreprocessVec(std::shared_ptr<RequestZcPacket>);

public:
   ZeroConfContainer(LMDBBlockDatabase* db,
      std::shared_ptr<BitcoinNodeInterface> node, unsigned maxZcThread) :
      db_(db), networkNode_(node), maxZcThreadCount_(maxZcThread)
   {
      zcEnabled_.store(false, std::memory_order_relaxed);

      zcPreprocessQueue_ = std::make_shared<PreprocessQueue>();

      //register ZC callbacks
      auto processInvTx = [this](std::vector<InvEntry> entryVec)->void
      {
         if (!zcEnabled_.load(std::memory_order_relaxed))
            return;
            
         auto payload = std::make_shared<ZcInvPayload>(false);
         payload->invVec_ = move(entryVec);
         zcWatcherQueue_.push_back(move(payload));
      };

      networkNode_->registerInvTxLambda(processInvTx);

      auto getTx = [this](std::unique_ptr<Payload> payload)->void
      {
         this->processTxGetDataReply(move(payload));
      };
      networkNode_->registerGetTxCallback(getTx);
   }

   bool hasTxByHash(const BinaryData& txHash) const;
   Tx getTxByHash(const BinaryData& txHash) const;

   void dropZC(std::shared_ptr<ZeroConfSharedStateSnapshot>, const BinaryDataRef&);
   void dropZC(std::shared_ptr<ZeroConfSharedStateSnapshot>, const std::set<BinaryData>&);

   void parseNewZC(ZcActionStruct);
   void parseNewZC(
      std::map<BinaryDataRef, std::shared_ptr<ParsedTx>> zcMap,
      std::shared_ptr<ZeroConfSharedStateSnapshot>,
      bool updateDB, bool notify,
      const std::pair<std::string, std::string>&,
      std::map<BinaryData, WatcherTxBody>&);
   bool isTxOutSpentByZC(const BinaryData& dbKey) const;

   void clear(void);

   std::map<BinaryData, std::shared_ptr<TxIOPair>>
      getUnspentZCforScrAddr(BinaryData scrAddr) const;
   std::map<BinaryData, std::shared_ptr<TxIOPair>>
      getRBFTxIOsforScrAddr(BinaryData scrAddr) const;

   std::vector<TxOut> getZcTxOutsForKey(const std::set<BinaryData>&) const;
   std::vector<UnspentTxOut> getZcUTXOsForKey(const std::set<BinaryData>&) const;

   void updateZCinDB(void);

   void handleInvTx();

   void init(std::shared_ptr<ScrAddrFilter>, bool clearMempool);
   void shutdown();

   void setWatcherNode(std::shared_ptr<BitcoinNodeInterface> watcherNode);
   void setZeroConfCallbacks(std::unique_ptr<ZeroConfCallbacks> ptr)
   {
      bdvCallbacks_ = std::move(ptr);
   }

   void broadcastZC(const std::vector<BinaryDataRef>& rawzc, 
      uint32_t timeout_ms, const ZcBroadcastCallback&,
      const std::string& bdvID, const std::string& requestID);
   bool setWatcherEntry(
      const BinaryData&, std::shared_ptr<BinaryData>, 
      const std::string&, const std::string&,
      std::map<std::string, std::string>&,
      bool watchEntry = true);

   bool isEnabled(void) const 
   { return zcEnabled_.load(std::memory_order_relaxed); }

   const std::map<BinaryData, std::shared_ptr<TxIOPair>>&
      getTxioMapForScrAddr(const BinaryData&) const;

   std::shared_ptr<ZeroConfSharedStateSnapshot> getSnapshot(void) const
   {
      auto ss = std::atomic_load_explicit(&snapshot_, std::memory_order_acquire);
      return ss;
   }

   std::shared_ptr<ParsedTx> getTxByKey(const BinaryData&) const;
   TxOut getTxOutCopy(const BinaryDataRef, unsigned) const;
   BinaryDataRef getKeyForHash(const BinaryDataRef&) const;
   BinaryDataRef getHashForKey(const BinaryDataRef&) const;

   std::shared_future<std::shared_ptr<ZcPurgePacket>> pushNewBlockNotification(
      Blockchain::ReorganizationState reorgState)
   { return actionQueue_->pushNewBlockNotification(reorgState); }

   BatchTxMap getBatchTxMap(
      std::shared_ptr<ZeroConfBatch>,
      std::shared_ptr<ZeroConfSharedStateSnapshot>);

   ZcActionQueue* const actionQueue(void) const { return actionQueue_.get(); }

   unsigned getMatcherMapSize(void) const 
   { 
      return actionQueue_->getMatcherMapSize(); 
   }
};

#endif