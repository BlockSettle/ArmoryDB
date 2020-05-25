////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Copyright (C) 2011-2015, Armory Technologies, Inc.                        //
//  Distributed under the GNU Affero General Public License (AGPL v3)         //
//  See LICENSE-ATI or http://www.gnu.org/licenses/agpl.html                  //
//                                                                            //
//                                                                            //
//  Copyright (C) 2016, goatpig                                               //            
//  Distributed under the MIT license                                         //
//  See LICENSE-MIT or https://opensource.org/licenses/MIT                    //                                   
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#include "TestUtils.h"
using namespace std;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// TODO:  These tests were taken directly from the BlockUtilsSuper.cpp where 
//        they previously ran without issue.  After bringing them over to here,
//        they now seg-fault.  Disabled for now, since the PartialMerkleTrees 
//        are not actually in use anywhere yet.
class DISABLED_PartialMerkleTest : public ::testing::Test
{
protected:

   virtual void SetUp(void)
   {
      vector<BinaryData> txList_(7);
      // The "abcd" quartets are to trigger endianness errors -- without them,
      // these hashes are palindromes that work regardless of your endian-handling
      txList_[0] = READHEX("00000000000000000000000000000000"
         "000000000000000000000000abcd0000");
      txList_[1] = READHEX("11111111111111111111111111111111"
         "111111111111111111111111abcd1111");
      txList_[2] = READHEX("22222222222222222222222222222222"
         "222222222222222222222222abcd2222");
      txList_[3] = READHEX("33333333333333333333333333333333"
         "333333333333333333333333abcd3333");
      txList_[4] = READHEX("44444444444444444444444444444444"
         "444444444444444444444444abcd4444");
      txList_[5] = READHEX("55555555555555555555555555555555"
         "555555555555555555555555abcd5555");
      txList_[6] = READHEX("66666666666666666666666666666666"
         "666666666666666666666666abcd6666");

      vector<BinaryData> merkleTree_ = BtcUtils::calculateMerkleTree(txList_);

      /*
      cout << "Merkle Tree looks like the following (7 tx): " << endl;
      cout << "The ** indicates the nodes we care about for partial tree test" << endl;
      cout << "                                                    \n";
      cout << "                   _____0a10_____                   \n";
      cout << "                  /              \\                  \n";
      cout << "                _/                \\_                \n";
      cout << "            65df                    b4d6            \n";
      cout << "          /      \\                /      \\          \n";
      cout << "      6971        22dc        5675        d0b6      \n";
      cout << "     /    \\      /    \\      /    \\      /          \n";
      cout << "   0000  1111  2222  3333  4444  5555  6666         \n";
      cout << "    **                            **                \n";
      cout << "    " << endl;
      cout << endl;

      cout << "Full Merkle Tree (this one has been unit tested before):" << endl;
      for(uint32_t i=0; i<merkleTree_.size(); i++)
      cout << "    " << i << " " << merkleTree_[i].toHexStr() << endl;
      */
   }

   vector<BinaryData> txList_;
   vector<BinaryData> merkleTree_;
};

////////////////////////////////////////////////////////////////////////////////
TEST_F(DISABLED_PartialMerkleTest, FullTree)
{
   vector<bool> isOurs(7);
   isOurs[0] = true;
   isOurs[1] = true;
   isOurs[2] = true;
   isOurs[3] = true;
   isOurs[4] = true;
   isOurs[5] = true;
   isOurs[6] = true;

   //cout << "Start serializing a full tree" << endl;
   PartialMerkleTree pmtFull(7, &isOurs, &txList_);
   BinaryData pmtSerFull = pmtFull.serialize();

   //cout << "Finished serializing (full)" << endl;
   //cout << "Merkle Root: " << pmtFull.getMerkleRoot().toHexStr() << endl;

   //cout << "Starting unserialize (full):" << endl;
   //cout << "Serialized: " << pmtSerFull.toHexStr() << endl;
   PartialMerkleTree pmtFull2(7);
   pmtFull2.unserialize(pmtSerFull);
   BinaryData pmtSerFull2 = pmtFull2.serialize();
   //cout << "Reserializ: " << pmtSerFull2.toHexStr() << endl;
   //cout << "Equal? " << (pmtSerFull==pmtSerFull2 ? "True" : "False") << endl;

   //cout << "Print Tree:" << endl;
   //pmtFull2.pprintTree();
   EXPECT_EQ(pmtSerFull, pmtSerFull2);
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(DISABLED_PartialMerkleTest, SingleLeaf)
{
   vector<bool> isOurs(7);
   /////////////////////////////////////////////////////////////////////////////
   // Test all 7 single-flagged trees
   for (uint32_t i = 0; i<7; i++)
   {
      for (uint32_t j = 0; j<7; j++)
         isOurs[j] = i == j;

      PartialMerkleTree pmt(7, &isOurs, &txList_);
      //cout << "Serializing (partial)" << endl;
      BinaryData pmtSer = pmt.serialize();
      PartialMerkleTree pmt2(7);
      //cout << "Unserializing (partial)" << endl;
      pmt2.unserialize(pmtSer);
      //cout << "Reserializing (partial)" << endl;
      BinaryData pmtSer2 = pmt2.serialize();
      //cout << "Serialized (Partial): " << pmtSer.toHexStr() << endl;
      //cout << "Reserializ (Partial): " << pmtSer.toHexStr() << endl;
      //cout << "Equal? " << (pmtSer==pmtSer2 ? "True" : "False") << endl;

      //cout << "Print Tree:" << endl;
      //pmt2.pprintTree();
      EXPECT_EQ(pmtSer, pmtSer2);
   }
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(DISABLED_PartialMerkleTest, MultiLeaf)
{
   // Use deterministic seed
   srand(0);

   vector<bool> isOurs(7);

   /////////////////////////////////////////////////////////////////////////////
   // Test a variety of 3-flagged trees
   for (uint32_t i = 0; i<512; i++)
   {
      if (i<256)
      {
         // 2/3 of leaves will be selected
         for (uint32_t j = 0; j<7; j++)
            isOurs[j] = (rand() % 3 < 2);
      }
      else
      {
         // 1/3 of leaves will be selected
         for (uint32_t j = 0; j<7; j++)
            isOurs[j] = (rand() % 3 < 1);
      }

      PartialMerkleTree pmt(7, &isOurs, &txList_);
      //cout << "Serializing (partial)" << endl;
      BinaryData pmtSer = pmt.serialize();
      PartialMerkleTree pmt2(7);
      //cout << "Unserializing (partial)" << endl;
      pmt2.unserialize(pmtSer);
      //cout << "Reserializing (partial)" << endl;
      BinaryData pmtSer2 = pmt2.serialize();
      //cout << "Serialized (Partial): " << pmtSer.toHexStr() << endl;
      //cout << "Reserializ (Partial): " << pmtSer.toHexStr() << endl;
      cout << "Equal? " << (pmtSer == pmtSer2 ? "True" : "False") << endl;

      //cout << "Print Tree:" << endl;
      //pmt2.pprintTree();
      EXPECT_EQ(pmtSer, pmtSer2);
   }
}


////////////////////////////////////////////////////////////////////////////////
TEST_F(DISABLED_PartialMerkleTest, EmptyTree)
{
   vector<bool> isOurs(7);
   isOurs[0] = false;
   isOurs[1] = false;
   isOurs[2] = false;
   isOurs[3] = false;
   isOurs[4] = false;
   isOurs[5] = false;
   isOurs[6] = false;

   //cout << "Start serializing a full tree" << endl;
   PartialMerkleTree pmtFull(7, &isOurs, &txList_);
   BinaryData pmtSerFull = pmtFull.serialize();

   //cout << "Finished serializing (full)" << endl;
   //cout << "Merkle Root: " << pmtFull.getMerkleRoot().toHexStr() << endl;

   //cout << "Starting unserialize (full):" << endl;
   //cout << "Serialized: " << pmtSerFull.toHexStr() << endl;
   PartialMerkleTree pmtFull2(7);
   pmtFull2.unserialize(pmtSerFull);
   BinaryData pmtSerFull2 = pmtFull2.serialize();
   //cout << "Reserializ: " << pmtSerFull2.toHexStr() << endl;
   //cout << "Equal? " << (pmtSerFull==pmtSerFull2 ? "True" : "False") << endl;

   //cout << "Print Tree:" << endl;
   //pmtFull2.pprintTree();
   EXPECT_EQ(pmtSerFull, pmtSerFull2);

}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class BlockUtilsSuper : public ::testing::Test
{
protected:
   BlockDataManagerThread *theBDMt_;
   Clients* clients_;

   void initBDM(void)
   {
      DBTestUtils::init();

      auto& magicBytes = NetworkConfig::getMagicBytes();

      auto nodePtr = make_shared<NodeUnitTest>(
         *(uint32_t*)magicBytes.getPtr(), false);
      auto watcherPtr = make_shared<NodeUnitTest>(
         *(uint32_t*)magicBytes.getPtr(), true);
      config.bitcoinNodes_ = make_pair(nodePtr, watcherPtr);
      config.rpcNode_ = make_shared<NodeRPC_UnitTest>(
         nodePtr, watcherPtr);

      theBDMt_ = new BlockDataManagerThread(config);
      iface_ = theBDMt_->bdm()->getIFace();

      nodePtr->setBlockchain(theBDMt_->bdm()->blockchain());
      nodePtr->setBlockFiles(theBDMt_->bdm()->blockFiles());

      auto mockedShutdown = [](void)->void {};
      clients_ = new Clients(theBDMt_, mockedShutdown);
   }

   /////////////////////////////////////////////////////////////////////////////
   virtual void SetUp()
   {
      LOGDISABLESTDOUT();
      zeros_ = READHEX("00000000");

      blkdir_ = string("./blkfiletest");
      homedir_ = string("./fakehomedir");
      ldbdir_ = string("./ldbtestdir");

      DBUtils::removeDirectory(blkdir_);
      DBUtils::removeDirectory(homedir_);
      DBUtils::removeDirectory(ldbdir_);

      mkdir(blkdir_);
      mkdir(homedir_);
      mkdir(ldbdir_);

      // Put the first 5 blocks into the blkdir
      BlockDataManagerConfig::setServiceType(SERVICE_UNITTEST);
      BlockDataManagerConfig::setOperationMode(OPERATION_UNITTEST);

      blk0dat_ = BtcUtils::getBlkFilename(blkdir_, 0);
      TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);

      BlockDataManagerConfig::setDbType(ARMORY_DB_SUPER);
      config.blkFileLocation_ = blkdir_;
      config.dbDir_ = ldbdir_;
      config.threadCount_ = 3;

      unsigned port_int = 50000 + rand() % 10000;
      stringstream port_ss;
      port_ss << port_int;
      config.listenPort_ = port_ss.str();

      initBDM();
   }

   /////////////////////////////////////////////////////////////////////////////
   virtual void TearDown(void)
   {
      if (clients_ != nullptr)
      {
         clients_->exitRequestLoop();
         clients_->shutdown();
      }

      delete clients_;
      delete theBDMt_;

      theBDMt_ = nullptr;
      clients_ = nullptr;
      
      DBUtils::removeDirectory(blkdir_);
      DBUtils::removeDirectory(homedir_);
      DBUtils::removeDirectory("./ldbtestdir");

      mkdir("./ldbtestdir");

      LOGENABLESTDOUT();
      CLEANUP_ALL_TIMERS();
   }

   BlockDataManagerConfig config;

   LMDBBlockDatabase* iface_;
   BinaryData zeros_;

   string blkdir_;
   string homedir_;
   string ldbdir_;
   string blk0dat_;
};

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsSuper, Load5Blocks)
{
   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   StoredScriptHistory ssh;

   BinaryData scrA(TestChain::scrAddrA);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 70 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 230 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 12);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 20 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 75 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 6);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 65 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 45 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 25 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsSuper, Load5Blocks_ReloadBDM)
{
   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   StoredScriptHistory ssh;

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 70 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 230 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 12);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 20 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 75 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 6);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 65 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 45 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 25 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   //restart bdm
   clients_->exitRequestLoop();
   clients_->shutdown();

   delete clients_;
   delete theBDMt_;

   initBDM();

   auto&& subssh_sdbi = iface_->getStoredDBInfo(SUBSSH, 0);
   EXPECT_EQ(subssh_sdbi.topBlkHgt_, 5);

   auto&& ssh_sdbi = iface_->getStoredDBInfo(SSH, 0);
   EXPECT_EQ(ssh_sdbi.topBlkHgt_, 5);

   theBDMt_->start(config.initMode_);
   bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 70 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 230 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 12);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 20 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 75 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 6);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 65 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 45 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 25 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsSuper, Load5Blocks_Reload_Rescan)
{
   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   StoredScriptHistory ssh;

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 70 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 230 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 12);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 20 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 75 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 6);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 65 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 45 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 25 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   //restart bdm
   clients_->exitRequestLoop();
   clients_->shutdown();

   delete clients_;
   delete theBDMt_;

   initBDM();

   auto&& subssh_sdbi = iface_->getStoredDBInfo(SUBSSH, 0);
   EXPECT_EQ(subssh_sdbi.topBlkHgt_, 5);

   auto&& ssh_sdbi = iface_->getStoredDBInfo(SSH, 0);
   EXPECT_EQ(ssh_sdbi.topBlkHgt_, 5);

   theBDMt_->start(INIT_RESCAN);
   bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 70 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 230 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 12);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 20 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 75 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 6);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 65 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 45 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 25 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsSuper, Load5Blocks_RescanSSH)
{
   TestUtils::setBlocks({ "0", "1", "2", "3" }, blk0dat_);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);


   StoredScriptHistory ssh;

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 160 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 9);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 55 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 55 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 5);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 10 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 10 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 10 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 20 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   //restart bdm
   clients_->exitRequestLoop();
   clients_->shutdown();

   delete clients_;
   delete theBDMt_;

   initBDM();

   auto&& subssh_sdbi = iface_->getStoredDBInfo(SUBSSH, 0);
   EXPECT_EQ(subssh_sdbi.topBlkHgt_, 3);

   auto&& ssh_sdbi = iface_->getStoredDBInfo(SSH, 0);
   EXPECT_EQ(ssh_sdbi.topBlkHgt_, 3);

   theBDMt_->start(INIT_SSH);
   bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 160 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 9);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 55 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 55 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 5);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 10 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 10 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 10 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 20 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   //restart bdm
   clients_->exitRequestLoop();
   clients_->shutdown();

   delete clients_;
   delete theBDMt_;


   initBDM();

   subssh_sdbi = iface_->getStoredDBInfo(SUBSSH, 0);
   EXPECT_EQ(subssh_sdbi.topBlkHgt_, 3);

   ssh_sdbi = iface_->getStoredDBInfo(SSH, 0);
   EXPECT_EQ(ssh_sdbi.topBlkHgt_, 3);

   //add next block
   TestUtils::appendBlocks({ "4" }, blk0dat_);

   theBDMt_->start(INIT_SSH);
   bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   subssh_sdbi = iface_->getStoredDBInfo(SUBSSH, 0);
   EXPECT_EQ(subssh_sdbi.topBlkHgt_, 4);

   ssh_sdbi = iface_->getStoredDBInfo(SSH, 0);
   EXPECT_EQ(ssh_sdbi.topBlkHgt_, 4);
   
   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 160 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 9);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 10 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 65 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 5);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 60 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 60 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 10 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 45 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 6);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 25 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   //add last block
   TestUtils::appendBlocks({ "5" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 70 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 230 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 12);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 20 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 75 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 6);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 65 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 45 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 25 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsSuper, Load3BlocksPlus3)
{
   // Copy only the first four blocks.  Will copy the full file next to test
   // readBlkFileUpdate method on non-reorg blocks.
   TestUtils::setBlocks({ "0", "1", "2" }, blk0dat_);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   EXPECT_EQ(DBTestUtils::getTopBlockHeight(iface_, HEADERS), 2);
   EXPECT_EQ(DBTestUtils::getTopBlockHash(iface_, HEADERS), TestChain::blkHash2);
   EXPECT_TRUE(theBDMt_->bdm()->blockchain()->
      getHeaderByHash(TestChain::blkHash2)->isMainBranch());

   TestUtils::appendBlocks({ "3" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   TestUtils::appendBlocks({ "5" }, blk0dat_);

   //restart bdm
   clients_->exitRequestLoop();
   clients_->shutdown();

   delete clients_;
   delete theBDMt_;

   initBDM();

   theBDMt_->start(config.initMode_);
   bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   TestUtils::appendBlocks({ "4" }, blk0dat_);

   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   EXPECT_EQ(DBTestUtils::getTopBlockHeight(iface_, HEADERS), 5);
   EXPECT_EQ(DBTestUtils::getTopBlockHash(iface_, HEADERS), TestChain::blkHash5);
   EXPECT_TRUE(theBDMt_->bdm()->blockchain()->
      getHeaderByHash(TestChain::blkHash5)->isMainBranch());

   StoredScriptHistory ssh;

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 70 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 230 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 12);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 20 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 75 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 6);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 65 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 45 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 25 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   //grab a tx by hash for coverage
   auto& txioHeightMap = ssh.subHistMap_.rbegin()->second;
   auto& txio = txioHeightMap.txioMap_.rbegin()->second;
   auto&& txhash = txio.getTxHashOfOutput(iface_);

   auto&& txObj = DBTestUtils::getTxByHash(clients_, bdvID, txhash);
   EXPECT_EQ(txObj.getThisHash(), txhash);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsSuper, Load5Blocks_FullReorg)
{
   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5", "4A", "5A" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   StoredScriptHistory ssh;

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 160 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 9);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 55 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 55 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 60 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 60 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 60 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 95 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 10 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 20 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsSuper, Load5Blocks_ReloadBDM_Reorg)
{
   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   //reload BDM
   clients_->exitRequestLoop();
   clients_->shutdown();

   delete theBDMt_;
   delete clients_;

   initBDM();

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5", "4A", "5A" }, blk0dat_);

   theBDMt_->start(config.initMode_);
   bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   EXPECT_EQ(theBDMt_->bdm()->blockchain()->top()->getBlockHeight(), 5);

   StoredScriptHistory ssh;

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 160 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 9);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 55 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 55 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 60 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 60 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 60 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 95 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 10 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 20 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsSuper, Load5Blocks_DoubleReorg)
{
   StoredScriptHistory ssh;

   TestUtils::setBlocks({ "0", "1", "2", "3", "4A" }, blk0dat_);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   //first reorg: up to 5
   TestUtils::setBlocks({ "0", "1", "2", "3", "4A", "4", "5" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 70 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 230 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 12);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 20 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 75 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 6);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 65 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 45 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 25 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 40 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   //second reorg: up to 5A
   TestUtils::setBlocks({ "0", "1", "2", "3", "4A", "4", "5", "5A" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 160 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 9);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 55 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 55 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 60 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 60 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 60 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 95 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb1ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 15 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddr);
   EXPECT_EQ(ssh.getScriptBalance(), 10 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 20 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 3);

   iface_->getStoredScriptHistory(ssh, TestChain::lb2ScrAddrP2SH);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 5 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsSuper, Load5Blocks_DynamicReorg_GrabSTXO)
{
   StoredScriptHistory ssh;
   TestUtils::setBlocks({ "0", "1", "2", "3" }, blk0dat_);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);

   //grab utxos at height 3
   auto utxosB = DBTestUtils::getUtxoForAddress(
      clients_, bdvID, TestChain::scrAddrB, false);

   auto utxosC = DBTestUtils::getUtxoForAddress(
      clients_, bdvID, TestChain::scrAddrC, false);

   //mine till block 5
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 50 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 50 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 1);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 70 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 230 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 12);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 20 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 75 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 6);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 65 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 30 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 30 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 2);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 45 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   //reorg from block 3
   {
      auto headerPtr = theBDMt_->bdm()->blockchain()->getHeaderByHeight(3, 0xFF);
      DBTestUtils::setReorgBranchingPoint(theBDMt_, headerPtr->getThisHash());
   }
  
   //instantiate resolver feed overloaded object
   auto feed = make_shared<ResolverUtils::TestResolverFeed>();

   auto addToFeed = [feed](const BinaryData& key)->void
   {
      auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
      feed->h160ToPubKey_.insert(datapair);
      feed->pubKeyToPrivKey_[datapair.second] = key;
   };

   addToFeed(TestChain::privKeyAddrB);
   addToFeed(TestChain::privKeyAddrC);
   addToFeed(TestChain::privKeyAddrD);
   addToFeed(TestChain::privKeyAddrE);
   addToFeed(TestChain::privKeyAddrF);

   /*create the transactions*/

   //grab utxo from raw tx lambda
   auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
   {
      Tx tx(rawTx);
      if (id > tx.getNumTxOut())
         throw runtime_error("invalid txout count");

      auto&& txOut = tx.getTxOutCopy(id);

      UTXO utxo;
      utxo.unserializeRaw(txOut.serialize());
      utxo.txOutIndex_ = id;
      utxo.txHash_ = tx.getThisHash();

      return utxo;
   };

   //2x 2 outputs
   BinaryData rawTx1, rawTx2;

   {
      //50 from B, 5 to A, change to D
      Signer signer;

      auto spender = make_shared<ScriptSpender>(utxosB[0]);
      signer.addSpender(spender);

      auto recA = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
      signer.addRecipient(recA);

      auto recChange = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrD.getSliceCopy(1, 20), 
         spender->getValue() - recA->getValue());
      signer.addRecipient(recChange);

      signer.setFeed(feed);
      signer.sign();
      rawTx1 = signer.serializeSignedTx();
   }

   {
      //50 from C, 10 to E, change to F
      Signer signer;

      auto spender = make_shared<ScriptSpender>(utxosC[0]);
      signer.addSpender(spender);

      auto recE = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
      signer.addRecipient(recE);

      auto recChange = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrF.getSliceCopy(1, 20), 
         spender->getValue() - recE->getValue());
      signer.addRecipient(recChange);

      signer.setFeed(feed);
      signer.sign();
      rawTx2 = signer.serializeSignedTx();
   }

   //4 outputs
   BinaryData rawTx3;
   {
      //45 from D, 40 from F, 6 to A, 7 to E, 8 to D, change to C
      auto zcUtxo1 = getUtxoFromRawTx(rawTx1, 1);
      auto zcUtxo2 = getUtxoFromRawTx(rawTx2, 1);

      Signer signer;
      
      auto spender1 = make_shared<ScriptSpender>(zcUtxo1);
      auto spender2 = make_shared<ScriptSpender>(zcUtxo2);
      signer.addSpender(spender1);
      signer.addSpender(spender2);

      auto recA = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrA.getSliceCopy(1, 20), 6 * COIN);
      signer.addRecipient(recA);

      auto recE = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrE.getSliceCopy(1, 20), 7 * COIN);
      signer.addRecipient(recE);
      
      auto recD = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrD.getSliceCopy(1, 20), 8 * COIN);
      signer.addRecipient(recD);

      auto recChange = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrC.getSliceCopy(1, 20),
         spender1->getValue() + spender2->getValue() - 
         recA->getValue() - recE->getValue() - recD->getValue());
      signer.addRecipient(recChange);

      signer.setFeed(feed);
      signer.sign();
      rawTx3 = signer.serializeSignedTx();
   }

   /*stage the transactions*/

   //2 tx with 2 outputs each in block 4
   DBTestUtils::ZcVector zcVec4;
   zcVec4.push_back(rawTx1, 10000000, 0);
   zcVec4.push_back(rawTx2, 11000000, 0);
   DBTestUtils::pushNewZc(theBDMt_, zcVec4, true);

   //no tx in block 5

   //1 tx with 4 outputs in block 6, cover the roundabout zc delay setter
   DBTestUtils::setNextZcPushDelay(2);
   DBTestUtils::ZcVector zcVec6;
   zcVec6.push_back(rawTx3, 20000000, 0);
   DBTestUtils::pushNewZc(theBDMt_, zcVec6, true);

   /*reorg*/
   DBTestUtils::mineNewBlock(theBDMt_, 
      TestChain::scrAddrA.getSliceCopy(1, 20), 3);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   /*check balances*/
   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrA);
   EXPECT_EQ(ssh.getScriptBalance(), 211 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 211 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 6);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrB);
   EXPECT_EQ(ssh.getScriptBalance(), 0 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 160 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 10);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrC);
   EXPECT_EQ(ssh.getScriptBalance(), 49 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 99 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 13 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 38 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrE);
   EXPECT_EQ(ssh.getScriptBalance(), 47 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 47 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 4);

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   EXPECT_EQ(ssh.getScriptReceived(), 80 * COIN);
   EXPECT_EQ(ssh.totalTxioCount_, 7);

   /*grab STXOs*/

   //block 4
   StoredTxOut stxo1, stxo2;
   auto&& key4_0_0_0 = DBUtils::getBlkDataKeyNoPrefix(4, 0, 0, 0);
   EXPECT_TRUE(iface_->getStoredTxOut(stxo1, key4_0_0_0));

   auto&& key4_1_0_0 = DBUtils::getBlkDataKeyNoPrefix(4, 1, 0, 0);
   EXPECT_TRUE(iface_->getStoredTxOut(stxo2, key4_1_0_0));
   EXPECT_NE(stxo1.dataCopy_, stxo2.dataCopy_);

   //block 5
   StoredTxOut stxo3, stxo4, stxo5;
   auto&& key5_0_0_0 = DBUtils::getBlkDataKeyNoPrefix(5, 0, 0, 0);
   EXPECT_TRUE(iface_->getStoredTxOut(stxo3, key5_0_0_0));

   auto&& key5_1_0_0 = DBUtils::getBlkDataKeyNoPrefix(5, 1, 0, 0);
   EXPECT_TRUE(iface_->getStoredTxOut(stxo4, key5_1_0_0));
   EXPECT_NE(stxo3.dataCopy_, stxo4.dataCopy_);

   auto&& key5_0_1_0 = DBUtils::getBlkDataKeyNoPrefix(5, 0, 1, 0);
   EXPECT_TRUE(iface_->getStoredTxOut(stxo5, key5_0_1_0));

   //block 6
   StoredTxOut stxo6, stxo7;
   auto&& key6_0_1_0 = DBUtils::getBlkDataKeyNoPrefix(6, 0, 1, 0);
   EXPECT_TRUE(iface_->getStoredTxOut(stxo6, key6_0_1_0));

   auto&& key6_1_1_0 = DBUtils::getBlkDataKeyNoPrefix(6, 1, 1, 0);
   EXPECT_FALSE(iface_->getStoredTxOut(stxo7, key6_1_1_0));
}

////////////////////////////////////////////////////////////////////////////////
// I thought I was going to do something different with this set of tests,
// but I ended up with an exact copy of the BlockUtilsSuper fixture.  Oh well.
class BlockUtilsWithWalletTest : public ::testing::Test
{
protected:
   BlockDataManagerThread *theBDMt_;
   Clients* clients_;

   void initBDM(void)
   {
      DBTestUtils::init();

      auto& magicBytes = NetworkConfig::getMagicBytes();

      auto nodePtr = make_shared<NodeUnitTest>(
         *(uint32_t*)magicBytes.getPtr(), false);
      auto watcherPtr = make_shared<NodeUnitTest>(
         *(uint32_t*)magicBytes.getPtr(), true);
      config.bitcoinNodes_ = make_pair(nodePtr, watcherPtr);
      config.rpcNode_ = make_shared<NodeRPC_UnitTest>(
         nodePtr, watcherPtr);

      theBDMt_ = new BlockDataManagerThread(config);
      iface_ = theBDMt_->bdm()->getIFace();

      nodePtr->setBlockchain(theBDMt_->bdm()->blockchain());
      nodePtr->setBlockFiles(theBDMt_->bdm()->blockFiles());
      nodePtr->setIface(iface_);

      auto mockedShutdown = [](void)->void {};
      clients_ = new Clients(theBDMt_, mockedShutdown);
   }

   /////////////////////////////////////////////////////////////////////////////
   virtual void SetUp()
   {
      LOGDISABLESTDOUT();
      zeros_ = READHEX("00000000");

      blkdir_ = string("./blkfiletest");
      homedir_ = string("./fakehomedir");
      ldbdir_ = string("./ldbtestdir");

      DBUtils::removeDirectory(blkdir_);
      DBUtils::removeDirectory(homedir_);
      DBUtils::removeDirectory(ldbdir_);

      mkdir(blkdir_);
      mkdir(homedir_);
      mkdir(ldbdir_);

      // Put the first 5 blocks into the blkdir
      BlockDataManagerConfig::setServiceType(SERVICE_UNITTEST);
      BlockDataManagerConfig::setOperationMode(OPERATION_UNITTEST);

      blk0dat_ = BtcUtils::getBlkFilename(blkdir_, 0);
      TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);

      BlockDataManagerConfig::setDbType(ARMORY_DB_SUPER);
      config.blkFileLocation_ = blkdir_;
      config.dbDir_ = ldbdir_;
      config.threadCount_ = 3;

      unsigned port_int = 50000 + rand() % 10000;
      stringstream port_ss;
      port_ss << port_int;
      config.listenPort_ = port_ss.str();

      initBDM();

      wallet1id = "wallet1";
   }

   /////////////////////////////////////////////////////////////////////////////
   virtual void TearDown(void)
   {
      if (clients_ != nullptr)
      {
         clients_->exitRequestLoop();
         clients_->shutdown();
      }

      delete clients_;
      delete theBDMt_;

      theBDMt_ = nullptr;
      clients_ = nullptr;

      DBUtils::removeDirectory(blkdir_);
      DBUtils::removeDirectory(homedir_);
      DBUtils::removeDirectory("./ldbtestdir");

      mkdir("./ldbtestdir");

      LOGENABLESTDOUT();
      CLEANUP_ALL_TIMERS();
      }

   BlockDataManagerConfig config;

   LMDBBlockDatabase* iface_;
   BinaryData zeros_;

   string blkdir_;
   string homedir_;
   string ldbdir_;
   string blk0dat_;

   string wallet1id;
};

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsWithWalletTest, Test_WithWallet)
{
   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());

   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");

   auto bdvPtr = DBTestUtils::getBDV(clients_, bdvID);

   //wait on signals
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);
   auto wlt = bdvPtr->getWalletOrLockbox(wallet1id);

   uint64_t balanceWlt;
   uint64_t balanceDB;

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrA)->getFullBalance();
   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrA);
   EXPECT_EQ(balanceWlt, 50 * COIN);
   EXPECT_EQ(balanceDB, 50 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrB)->getFullBalance();
   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrB);
   EXPECT_EQ(balanceWlt, 70 * COIN);
   EXPECT_EQ(balanceDB, 70 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrC)->getFullBalance();
   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrC);
   EXPECT_EQ(balanceWlt, 20 * COIN);
   EXPECT_EQ(balanceDB, 20 * COIN);

   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrD);
   EXPECT_EQ(balanceDB, 65 * COIN);
   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrE);
   EXPECT_EQ(balanceDB, 30 * COIN);
   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrF);
   EXPECT_EQ(balanceDB, 5 * COIN);   
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsWithWalletTest, RegisterAddrAfterWallet)
{
   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());

   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");
   auto bdvPtr = DBTestUtils::getBDV(clients_, bdvID);

   //wait on signals
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);
   auto wlt = bdvPtr->getWalletOrLockbox(wallet1id);

   uint64_t balanceWlt;
   uint64_t balanceDB;

   //post initial load address registration
   scrAddrVec.clear();
   scrAddrVec.push_back(TestChain::scrAddrD);
   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrA)->getFullBalance();
   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrA);
   EXPECT_EQ(balanceWlt, 50 * COIN);
   EXPECT_EQ(balanceDB, 50 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrB)->getFullBalance();
   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrB);
   EXPECT_EQ(balanceWlt, 70 * COIN);
   EXPECT_EQ(balanceDB, 70 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrC)->getFullBalance();
   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrC);
   EXPECT_EQ(balanceWlt, 20 * COIN);
   EXPECT_EQ(balanceDB, 20 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrD)->getFullBalance();
   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrD);
   EXPECT_EQ(balanceWlt, 65 * COIN);
   EXPECT_EQ(balanceDB, 65 * COIN);

   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrE);
   EXPECT_EQ(balanceDB, 30 * COIN);
   balanceDB = iface_->getBalanceForScrAddr(TestChain::scrAddrF);
   EXPECT_EQ(balanceDB, 5 * COIN);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsWithWalletTest, ZeroConfUpdate)
{
   //create script spender objects
   auto getSpenderPtr = [](
      const UnspentTxOut& utxo,
      shared_ptr<ResolverFeed> feed)
      ->shared_ptr<ScriptSpender>
   {
      UTXO entry(utxo.value_, utxo.txHeight_, utxo.txIndex_, utxo.txOutIndex_,
         move(utxo.txHash_), move(utxo.script_));

      auto spender = make_shared<ScriptSpender>(entry, feed);
      spender->setSequence(UINT32_MAX - 2);

      return spender;
   };

   TestUtils::setBlocks({ "0", "1", "2", "3" }, blk0dat_);

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);
   scrAddrVec.push_back(TestChain::scrAddrE);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());

   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");

   auto bdvPtr = DBTestUtils::getBDV(clients_, bdvID);

   //wait on signals
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);
   auto wlt = bdvPtr->getWalletOrLockbox(wallet1id);

   BinaryData ZChash;

   {
      ////spend 27 from wlt to assetWlt's first 2 unused addresses
      ////send rest back to scrAddrA

      auto spendVal = 27 * COIN;
      Signer signer;
      signer.setLockTime(3);

      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);

      //get utxo list for spend value
      auto&& unspentVec = wlt->getSpendableTxOutListForValue(spendVal);

      vector<UnspentTxOut> utxoVec;
      uint64_t tval = 0;
      auto utxoIter = unspentVec.begin();
      while (utxoIter != unspentVec.end())
      {
         tval += utxoIter->getValue();
         utxoVec.push_back(*utxoIter);

         if (tval > spendVal)
            break;

         ++utxoIter;
      }

      //create script spender objects
      uint64_t total = 0;
      for (auto& utxo : utxoVec)
      {
         total += utxo.getValue();
         signer.addSpender(getSpenderPtr(utxo, feed));
      }

      //spendVal to addrE
      auto recipientChange = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrD.getSliceCopy(1, 20), spendVal);
      signer.addRecipient(recipientChange);

      if (total > spendVal)
      {
         //change to scrAddrD, no fee
         auto changeVal = total - spendVal;
         auto recipientChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), changeVal);
         signer.addRecipient(recipientChange);
      }

      //sign, verify then broadcast
      signer.sign();
      EXPECT_TRUE(signer.verify());

      Tx zctx(signer.serializeSignedTx());
      ZChash = zctx.getThisHash();

      DBTestUtils::ZcVector zcVec;
      zcVec.push_back(signer.serializeSignedTx(), 1300000000);

      DBTestUtils::pushNewZc(theBDMt_, zcVec);
      DBTestUtils::waitOnNewZcSignal(clients_, bdvID);
   }

   EXPECT_EQ(wlt->getScrAddrObjByKey(TestChain::scrAddrA)->getFullBalance(), 50 * COIN);
   EXPECT_EQ(wlt->getScrAddrObjByKey(TestChain::scrAddrB)->getFullBalance(), 30 * COIN);
   EXPECT_EQ(wlt->getScrAddrObjByKey(TestChain::scrAddrC)->getFullBalance(), 55 * COIN);
   EXPECT_EQ(wlt->getScrAddrObjByKey(TestChain::scrAddrE)->getFullBalance(), 3 * COIN);

   //test ledger entry
   LedgerEntry le = DBTestUtils::getLedgerEntryFromWallet(wlt, ZChash);

   //EXPECT_EQ(le.getTxTime(), 1300000000);
   EXPECT_EQ(le.isSentToSelf(), false);
   EXPECT_EQ(le.getValue(), -27 * COIN);

   //check ZChash in DB
   BinaryData zcKey = WRITE_UINT16_BE(0xFFFF);
   zcKey.append(WRITE_UINT32_LE(0));
   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getHashForKey(zcKey), ZChash);

   //grab ZC by hash
   auto&& txobj = DBTestUtils::getTxByHash(clients_, bdvID, ZChash);
   EXPECT_EQ(txobj.getThisHash(), ZChash);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsWithWalletTest, UnrelatedZC_CheckLedgers)
{
   TestUtils::setBlocks({ "0", "1", "2", "3", "4" }, blk0dat_);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);
   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");

   auto bdvPtr = DBTestUtils::getBDV(clients_, bdvID);

   //wait on signals
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);
   auto wlt = bdvPtr->getWalletOrLockbox(wallet1id);
   auto delegateID = DBTestUtils::getLedgerDelegate(clients_, bdvID);

   //check balances
   const ScrAddrObj* scrObj;
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 10 * COIN);

   StoredScriptHistory ssh;
   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 60 * COIN);
   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 10 * COIN);

   //Create zc that spends from addr D to F. This is supernode so the DB
   //should track this ZC even though it isn't registered. Send the ZC as
   //a batch along with a ZC that hits our wallets, in order to get the 
   //notification, which comes at the BDV level (i.e. only for registered
   //wallets).
   
   auto&& ZC1 = TestUtils::getTx(5, 2); //block 5, tx 2
   auto&& ZChash1 = BtcUtils::getHash256(ZC1);

   auto&& ZC2 = TestUtils::getTx(5, 1); //block 5, tx 1
   auto&& ZChash2 = BtcUtils::getHash256(ZC2);

   DBTestUtils::ZcVector zcVec1;
   zcVec1.push_back(ZC1, 14000000);
   zcVec1.push_back(ZC2, 14100000);

   DBTestUtils::pushNewZc(theBDMt_, zcVec1);
   DBTestUtils::waitOnNewZcSignal(clients_, bdvID);

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 20 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 20 * COIN);

   try
   {
      auto zcTxios = theBDMt_->bdm()->zeroConfCont()->getTxioMapForScrAddr(
            TestChain::scrAddrD);
      EXPECT_EQ(zcTxios.size(), 1);
      iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
      DBTestUtils::addTxioToSsh(ssh, zcTxios);
      EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);
   }
   catch (exception&)
   {
      ASSERT_TRUE(false);
   }

   try
   {
      auto zcTxios = theBDMt_->bdm()->zeroConfCont()->getTxioMapForScrAddr(
            TestChain::scrAddrF);
      iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
      DBTestUtils::addTxioToSsh(ssh, zcTxios);
      EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   }
   catch (exception&)
   {
      ASSERT_TRUE(false);
   }

   //grab ledger for 1st ZC, should be empty
   auto zcledger = DBTestUtils::getLedgerEntryFromWallet(wlt, ZChash1);
   EXPECT_EQ(zcledger.getTxHash(), BtcUtils::EmptyHash());

   //grab ledger for 2nd ZC
   zcledger = DBTestUtils::getLedgerEntryFromWallet(wlt, ZChash2);
   EXPECT_EQ(zcledger.getValue(), 30 * COIN);
   EXPECT_EQ(zcledger.getBlockNum(), UINT32_MAX);
   EXPECT_FALSE(zcledger.isOptInRBF());

   //grab delegate ledger
   auto&& delegateLedger = 
      DBTestUtils::getHistoryPage(clients_, bdvID, delegateID, 0);

   unsigned zc2_count = 0;
   for (auto& ld : delegateLedger)
   {
      if (ld.getTxHash() == ZChash2)
         zc2_count++;
   }

   EXPECT_EQ(zc2_count, 1);

   //push last block
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 70 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 20 * COIN);

   try
   {
      auto zcTxios = theBDMt_->bdm()->zeroConfCont()->getTxioMapForScrAddr(
         TestChain::scrAddrD);
      ASSERT_TRUE(false);
   }
   catch (exception&)
   {}
   
   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);

   try
   {   
      auto zcTxios = theBDMt_->bdm()->zeroConfCont()->getTxioMapForScrAddr(
            TestChain::scrAddrF);
      ASSERT_TRUE(false);
   }
   catch (exception&)
   {}

   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);

   //try to get ledgers, ZCs should be all gone
   zcledger = DBTestUtils::getLedgerEntryFromWallet(wlt, ZChash1);
   EXPECT_EQ(zcledger.getTxHash(), BtcUtils::EmptyHash());
   
   zcledger = DBTestUtils::getLedgerEntryFromWallet(wlt, ZChash2);
   EXPECT_EQ(zcledger.getTxTime(), 1231009513);
   EXPECT_EQ(zcledger.getBlockNum(), 5);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsWithWalletTest, RegisterAfterZC)
{
   TestUtils::setBlocks({ "0", "1", "2", "3", "4" }, blk0dat_);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);
   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");

   auto bdvPtr = DBTestUtils::getBDV(clients_, bdvID);

   //wait on signals
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);
   auto wlt = bdvPtr->getWalletOrLockbox(wallet1id);
   auto delegateID = DBTestUtils::getLedgerDelegate(clients_, bdvID);

   //check balances
   const ScrAddrObj* scrObj;
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 10 * COIN);

   StoredScriptHistory ssh;
   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
   EXPECT_EQ(ssh.getScriptBalance(), 60 * COIN);
   iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
   EXPECT_EQ(ssh.getScriptBalance(), 10 * COIN);

   //Create zc that spends from addr D to F. This is supernode so the DB
   //should track this ZC even though it isn't registered. Send the ZC as
   //a batch along with a ZC that hits our wallets, in order to get the
   //notification, which comes at the BDV level (i.e. only for registered
   //wallets).

   auto&& ZC1 = TestUtils::getTx(5, 2); //block 5, tx 2
   auto&& ZChash1 = BtcUtils::getHash256(ZC1);

   auto&& ZC2 = TestUtils::getTx(5, 1); //block 5, tx 1
   auto&& ZChash2 = BtcUtils::getHash256(ZC2);

   DBTestUtils::ZcVector zcVec1;
   zcVec1.push_back(ZC1, 14000000);
   zcVec1.push_back(ZC2, 14100000);

   DBTestUtils::pushNewZc(theBDMt_, zcVec1);
   DBTestUtils::waitOnNewZcSignal(clients_, bdvID);

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 20 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 20 * COIN);

   try
   {
      auto zcTxios = theBDMt_->bdm()->zeroConfCont()->getTxioMapForScrAddr(
            TestChain::scrAddrD);
      iface_->getStoredScriptHistory(ssh, TestChain::scrAddrD);
      DBTestUtils::addTxioToSsh(ssh, zcTxios);
      EXPECT_EQ(ssh.getScriptBalance(), 65 * COIN);
   }
   catch (exception&)
   {
      ASSERT_TRUE(false);
   }

   try
   {
      auto zcTxios = theBDMt_->bdm()->zeroConfCont()->getTxioMapForScrAddr(
            TestChain::scrAddrF);
      iface_->getStoredScriptHistory(ssh, TestChain::scrAddrF);
      DBTestUtils::addTxioToSsh(ssh, zcTxios);
      EXPECT_EQ(ssh.getScriptBalance(), 5 * COIN);
   }
   catch (exception&)
   {
      ASSERT_TRUE(false);
   }

   //Register scrAddrD with the wallet. It should have the ZC balance
   scrAddrVec.push_back(TestChain::scrAddrD);
   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");
   
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrD);
   EXPECT_EQ(scrObj->getFullBalance(), 65 * COIN);

   //add last block
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 70 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 20 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrD);
   EXPECT_EQ(scrObj->getFullBalance(), 65 * COIN);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsWithWalletTest, ZC_Reorg)
{
   //create spender lamba
   auto getSpenderPtr = [](
      const UnspentTxOut& utxo,
      shared_ptr<ResolverFeed> feed)
      ->shared_ptr<ScriptSpender>
   {
      UTXO entry(utxo.value_, utxo.txHeight_, utxo.txIndex_, utxo.txOutIndex_,
         move(utxo.txHash_), move(utxo.script_));

      return make_shared<ScriptSpender>(entry, feed);
   };

   //
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());

   auto&& wltRoot = CryptoPRNG::generateRandom(32);
   auto assetWlt = AssetWallet_Single::createFromPrivateRoot_Armory135(
      homedir_,
      move(wltRoot), //root as a rvalue
      {},
      SecureBinaryData(),
      SecureBinaryData(),
      3); //set lookup computation to 3 entries
   auto addr1_ptr = assetWlt->getNewAddress();
   auto addr2_ptr = assetWlt->getNewAddress();

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);
   
   auto&& wltSet = assetWlt->getAddrHashSet();
   vector<BinaryData> wltVec;
   for (auto& addr : wltSet)
      wltVec.push_back(addr);

   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");
   DBTestUtils::registerWallet(clients_, bdvID, wltVec, assetWlt->getID());
   auto bdvPtr = DBTestUtils::getBDV(clients_, bdvID);

   //wait on signals
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);
   auto wlt = bdvPtr->getWalletOrLockbox(wallet1id);
   auto assetWltDbObj = bdvPtr->getWalletOrLockbox(assetWlt->getID());
   auto delegateID = DBTestUtils::getLedgerDelegate(clients_, bdvID);

   //check balances
   const ScrAddrObj* scrObj;
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 70 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 20 * COIN);

   BinaryData ZCHash1, ZCHash2;
   for (auto& sa : wltSet)
   {
      scrObj = assetWltDbObj->getScrAddrObjByKey(sa);
      EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
   }

   {
      Signer signer;

      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //get utxo list for spend value
      auto&& unspentVec = wlt->getSpendableTxOutListForValue(UINT64_MAX);

      //consume 1st utxo, send 2 to scrAddrA, 3 to new wallet
      signer.addSpender(getSpenderPtr(unspentVec[0], feed));
      signer.addRecipient(addr1_ptr->getRecipient(3 * COIN));
      auto recipientChange = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrA.getSliceCopy(1, 20), 2 * COIN);
      signer.addRecipient(recipientChange);
      signer.sign();

      //2nd tx, 2nd utxo, 5 to scrAddrB, 5 new wallet
      Signer signer2;
      signer2.addSpender(getSpenderPtr(unspentVec[1], feed));
      signer2.addRecipient(addr2_ptr->getRecipient(5 * COIN));
      auto recipientChange2 = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrB.getSliceCopy(1, 20), 5 * COIN);
      signer2.addRecipient(recipientChange2);
      signer2.sign();

      DBTestUtils::ZcVector zcVec;
      zcVec.push_back(signer.serializeSignedTx(), 14000000);
      ZCHash1 = zcVec.zcVec_.back().first.getThisHash();

      zcVec.push_back(signer2.serializeSignedTx(), 14100000);
      ZCHash2 = zcVec.zcVec_.back().first.getThisHash();

      DBTestUtils::pushNewZc(theBDMt_, zcVec);
      DBTestUtils::waitOnNewZcSignal(clients_, bdvID);
   }

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 52 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 75 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);

   scrObj = assetWltDbObj->getScrAddrObjByKey(addr1_ptr->getPrefixedHash());
   EXPECT_EQ(scrObj->getFullBalance(), 3 * COIN);
   scrObj = assetWltDbObj->getScrAddrObjByKey(addr2_ptr->getPrefixedHash());
   EXPECT_EQ(scrObj->getFullBalance(), 5 * COIN);

   //reorg the chain
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5", "4A", "5A" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   auto&& newBlockNotif = DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);
   
   //check new block callback carries an invalidated zc notif as well
   auto notifPtr = get<0>(newBlockNotif);
   auto notifIndex = get<1>(newBlockNotif);

   EXPECT_EQ(notifIndex, 0);
   EXPECT_EQ(notifPtr->notification_size(), 2);

   //grab the invalidated zc notif, it should carry the hash for both our ZC
   auto& zcNotif = notifPtr->notification(1);
   EXPECT_EQ(zcNotif.type(), ::Codec_BDVCommand::NotificationType::invalidated_zc);
   EXPECT_TRUE(zcNotif.has_ids());
   
   auto& ids = zcNotif.ids();
   EXPECT_EQ(ids.value_size(), 2);
   
   //check zc hash 1
   auto& id0_str = ids.value(0).data();
   BinaryData id0_bd((uint8_t*)id0_str.c_str(), id0_str.size());
   EXPECT_EQ(ZCHash1, id0_bd);

   //check zc hash 2
   auto& id1_str = ids.value(1).data();
   BinaryData id1_bd((uint8_t*)id1_str.c_str(), id1_str.size());
   EXPECT_EQ(ZCHash2, id1_bd);


   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 55 * COIN);

   scrObj = assetWltDbObj->getScrAddrObjByKey(addr1_ptr->getPrefixedHash());
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
   scrObj = assetWltDbObj->getScrAddrObjByKey(addr2_ptr->getPrefixedHash());
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsWithWalletTest, MultipleSigners_2of3_NativeP2WSH)
{
   //create spender lamba
   auto getSpenderPtr = [](
      const UnspentTxOut& utxo,
      shared_ptr<ResolverFeed> feed)
      ->shared_ptr<ScriptSpender>
   {
      UTXO entry(utxo.value_, utxo.txHeight_, utxo.txIndex_, utxo.txOutIndex_,
         move(utxo.txHash_), move(utxo.script_));

      return make_shared<ScriptSpender>(entry, feed);
   };

   //
   TestUtils::setBlocks({ "0", "1", "2", "3" }, blk0dat_);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);
   scrAddrVec.push_back(TestChain::scrAddrD);
   scrAddrVec.push_back(TestChain::scrAddrE);

   //// create 3 assetWlt ////

   //create a root private key
   auto&& wltRoot = CryptoPRNG::generateRandom(32);
   auto assetWlt_1 = AssetWallet_Single::createFromPrivateRoot_Armory135(
      homedir_,
      move(wltRoot), //root as a rvalue
      {},
      SecureBinaryData(),
      SecureBinaryData(), 
      3); //set lookup computation to 3 entries

   wltRoot = move(CryptoPRNG::generateRandom(32));
   auto assetWlt_2 = AssetWallet_Single::createFromPrivateRoot_Armory135(
      homedir_,
      move(wltRoot), //root as a rvalue
      {},
      SecureBinaryData(),
      SecureBinaryData(), 
      3); //set lookup computation to 3 entries

   wltRoot = move(CryptoPRNG::generateRandom(32));
   auto assetWlt_3 = AssetWallet_Single::createFromPrivateRoot_Armory135(
      homedir_,
      move(wltRoot), //root as a rvalue
      {},
      SecureBinaryData(),
      SecureBinaryData(), 
      3); //set lookup computation to 3 entries

          //create 2-of-3 multisig asset entry from 3 different wallets
   map<BinaryData, shared_ptr<AssetEntry>> asset_single_map;
   auto asset1 = assetWlt_1->getMainAccountAssetForIndex(0);
   auto wltid1_bd = assetWlt_1->getID();
   asset_single_map.insert(make_pair(BinaryData::fromString(wltid1_bd), asset1));

   auto asset2 = assetWlt_2->getMainAccountAssetForIndex(0);
   auto wltid2_bd = assetWlt_2->getID();
   asset_single_map.insert(make_pair(BinaryData::fromString(wltid2_bd), asset2));

   auto asset4_singlesig = assetWlt_2->getNewAddress();

   auto asset3 = assetWlt_3->getMainAccountAssetForIndex(0);
   auto wltid3_bd = assetWlt_3->getID();
   asset_single_map.insert(make_pair(BinaryData::fromString(wltid3_bd), asset3));

   auto ae_ms = make_shared<AssetEntry_Multisig>(0, BinaryData::fromString("test"),
      asset_single_map, 2, 3);
   auto addr_ms_raw = make_shared<AddressEntry_Multisig>(ae_ms, true);
   auto addr_p2wsh = make_shared<AddressEntry_P2WSH>(addr_ms_raw);


   //register with db
   vector<BinaryData> addrVec;
   addrVec.push_back(addr_p2wsh->getPrefixedHash());

   vector<BinaryData> addrVec_singleSig;
   auto&& addrSet = assetWlt_2->getAddrHashSet();
   for (auto& addr : addrSet)
      addrVec_singleSig.push_back(addr);

   DBTestUtils::registerWallet(clients_, bdvID, addrVec, "ms_entry");
   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");
   DBTestUtils::registerWallet(clients_, bdvID, addrVec_singleSig, assetWlt_2->getID());

   auto bdvPtr = DBTestUtils::getBDV(clients_, bdvID);

   //wait on signals
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);
   auto wlt = bdvPtr->getWalletOrLockbox(wallet1id);
   auto ms_wlt = bdvPtr->getWalletOrLockbox("ms_entry");
   auto wlt_singleSig = bdvPtr->getWalletOrLockbox(assetWlt_2->getID());


   //check balances
   const ScrAddrObj* scrObj;
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 55 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrD);
   EXPECT_EQ(scrObj->getFullBalance(), 5 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrE);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);

   //check new wallet balances
   scrObj = ms_wlt->getScrAddrObjByKey(addrVec[0]);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);

   {
      ////spend 27 from wlt to ms_wlt only address
      ////send rest back to scrAddrA

      auto spendVal = 27 * COIN;
      Signer signer;

      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);

      //get utxo list for spend value
      auto&& unspentVec = wlt->getSpendableTxOutListForValue(spendVal);

      vector<UnspentTxOut> utxoVec;
      uint64_t tval = 0;
      auto utxoIter = unspentVec.begin();
      while (utxoIter != unspentVec.end())
      {
         tval += utxoIter->getValue();
         utxoVec.push_back(*utxoIter);

         if (tval > spendVal)
            break;

         ++utxoIter;
      }

      //create script spender objects
      uint64_t total = 0;
      for (auto& utxo : utxoVec)
      {
         total += utxo.getValue();
         signer.addSpender(getSpenderPtr(utxo, feed));
      }

      //spend 20 to nested p2wsh script hash
      signer.addRecipient(addr_p2wsh->getRecipient(20 * COIN));

      //spend 7 to assetWlt_2
      signer.addRecipient(asset4_singlesig->getRecipient(7 * COIN));

      if (total > spendVal)
      {
         //change to scrAddrD, no fee
         auto changeVal = total - spendVal;
         auto recipientChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), changeVal);
         signer.addRecipient(recipientChange);
      }

      //sign, verify then broadcast
      signer.sign();
      EXPECT_TRUE(signer.verify());
      auto&& zcHash = signer.getTxId();

      DBTestUtils::ZcVector zcVec;
      zcVec.push_back(signer.serializeSignedTx(), 14000000);

      DBTestUtils::pushNewZc(theBDMt_, zcVec);
      DBTestUtils::waitOnNewZcSignal(clients_, bdvID);

      //grab ZC from DB and verify it again
      auto&& zc_from_db = DBTestUtils::getTxByHash(clients_, bdvID, zcHash);
      auto&& raw_tx = zc_from_db.serialize();
      auto bctx = BCTX::parse(raw_tx);
      TransactionVerifier tx_verifier(*bctx, utxoVec);

      ASSERT_TRUE(tx_verifier.evaluateState().isValid());
   }

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 55 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrD);
   EXPECT_EQ(scrObj->getFullBalance(), 8 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrE);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);

   //check new wallet balances
   scrObj = ms_wlt->getScrAddrObjByKey(addrVec[0]);
   EXPECT_EQ(scrObj->getFullBalance(), 20 * COIN);
   scrObj = wlt_singleSig->getScrAddrObjByKey(asset4_singlesig->getPrefixedHash());
   EXPECT_EQ(scrObj->getFullBalance(), 7 * COIN);

   auto spendVal = 18 * COIN;
   Signer signer2;
   signer2.setFlags(SCRIPT_VERIFY_SEGWIT);

   //get utxo list for spend value
   auto&& unspentVec =
      ms_wlt->getSpendableTxOutListZC();

   auto&& unspentVec_singleSig = wlt_singleSig->getSpendableTxOutListZC();

   unspentVec.insert(unspentVec.end(),
      unspentVec_singleSig.begin(), unspentVec_singleSig.end());

   //create feed from asset wallet 1
   auto feed_ms = make_shared<ResolverFeed_AssetWalletSingle_ForMultisig>(assetWlt_1);
   auto assetFeed = make_shared<ResolverUtils::CustomFeed>(addr_p2wsh, feed_ms);

   //create spenders
   uint64_t total = 0;
   for (auto& utxo : unspentVec)
   {
      total += utxo.getValue();
      signer2.addSpender(getSpenderPtr(utxo, assetFeed));
   }

   //creates outputs
   //spend 18 to addr 0, use P2PKH
   auto recipient2 = make_shared<Recipient_P2PKH>(
      TestChain::scrAddrB.getSliceCopy(1, 20), spendVal);
   signer2.addRecipient(recipient2);

   if (total > spendVal)
   {
      //deal with change, no fee
      auto changeVal = total - spendVal;
      signer2.addRecipient(addr_p2wsh->getRecipient(changeVal));
   }

   //sign, verify & return signed tx
   signer2.resolveSpenders();
   auto&& signerState = signer2.evaluateSignedState();

   {
      ASSERT_EQ(signerState.getEvalMapSize(), 2);

      auto&& txinEval = signerState.getSignedStateForInput(0);
      auto& pubkeyMap = txinEval.getPubKeyMap();
      EXPECT_EQ(pubkeyMap.size(), 3);
      for (auto& pubkeyState : pubkeyMap)
         EXPECT_FALSE(pubkeyState.second);

      txinEval = signerState.getSignedStateForInput(1);
      auto& pubkeyMap_2 = txinEval.getPubKeyMap();
      EXPECT_EQ(pubkeyMap_2.size(), 0);
   }

   {
      auto lock = assetWlt_1->lockDecryptedContainer();
      signer2.sign();
   }

   EXPECT_FALSE(signer2.verify());

   {
      //signer state with 1 sig
      EXPECT_FALSE(signer2.isSigned());
      signerState = signer2.evaluateSignedState();

      EXPECT_EQ(signerState.getEvalMapSize(), 2);

      auto&& txinEval = signerState.getSignedStateForInput(0);
      EXPECT_EQ(txinEval.getSigCount(), 1);

      auto asset_single = dynamic_pointer_cast<AssetEntry_Single>(asset1);
      ASSERT_NE(asset_single, nullptr);
      ASSERT_TRUE(txinEval.isSignedForPubKey(asset_single->getPubKey()->getCompressedKey()));
   }

   Signer signer3;
   //create feed from asset wallet 2
   auto feed_ms3 = make_shared<ResolverFeed_AssetWalletSingle_ForMultisig>(assetWlt_2);
   auto assetFeed3 = make_shared<ResolverUtils::CustomFeed>(addr_p2wsh, feed_ms3);
   signer3.deserializeState(signer2.serializeState());

   {
      //make sure sig was properly carried over with state
      EXPECT_FALSE(signer3.isSigned());
      signerState = signer3.evaluateSignedState();

      EXPECT_EQ(signerState.getEvalMapSize(), 2);
      auto&& txinEval = signerState.getSignedStateForInput(0);
      EXPECT_EQ(txinEval.getSigCount(), 1);

      auto asset_single = dynamic_pointer_cast<AssetEntry_Single>(asset1);
      ASSERT_NE(asset_single, nullptr);
      ASSERT_TRUE(txinEval.isSignedForPubKey(asset_single->getPubKey()->getCompressedKey()));
   }

   signer3.setFeed(assetFeed3);

   {
      auto lock = assetWlt_2->lockDecryptedContainer();
      signer3.sign();
   }

   {
      auto assetFeed4 = make_shared<ResolverFeed_AssetWalletSingle>(assetWlt_2);
      signer3.resetFeeds();
      signer3.setFeed(assetFeed4);
      auto lock = assetWlt_2->lockDecryptedContainer();
      signer3.sign();
   }


   ASSERT_TRUE(signer3.isSigned());
   try
   {
      signer3.verify();
   }
   catch (...)
   {
      EXPECT_TRUE(false);
   }

   {
      //should have 2 sigs now
      EXPECT_TRUE(signer3.isSigned());
      signerState = signer3.evaluateSignedState();

      EXPECT_EQ(signerState.getEvalMapSize(), 2);
      auto&& txinEval = signerState.getSignedStateForInput(0);
      EXPECT_EQ(txinEval.getSigCount(), 2);

      auto asset_single = dynamic_pointer_cast<AssetEntry_Single>(asset1);
      ASSERT_NE(asset_single, nullptr);
      ASSERT_TRUE(txinEval.isSignedForPubKey(asset_single->getPubKey()->getCompressedKey()));

      asset_single = dynamic_pointer_cast<AssetEntry_Single>(asset2);
      ASSERT_NE(asset_single, nullptr);
      ASSERT_TRUE(txinEval.isSignedForPubKey(asset_single->getPubKey()->getCompressedKey()));
   }

   auto&& tx1 = signer3.serializeSignedTx();
   auto&& zcHash = signer3.getTxId();

   //broadcast the last one
   DBTestUtils::ZcVector zcVec;
   zcVec.push_back(tx1, 15000000);

   DBTestUtils::pushNewZc(theBDMt_, zcVec);
   DBTestUtils::waitOnNewZcSignal(clients_, bdvID);

   //grab ZC from DB and verify it again
   auto&& zc_from_db = DBTestUtils::getTxByHash(clients_, bdvID, zcHash);
   auto&& raw_tx = zc_from_db.serialize();
   auto bctx = BCTX::parse(raw_tx);
   TransactionVerifier tx_verifier(*bctx, unspentVec);

   ASSERT_TRUE(tx_verifier.evaluateState().isValid());


   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 48 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 55 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrD);
   EXPECT_EQ(scrObj->getFullBalance(), 8 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrE);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);

   //check new wallet balances
   scrObj = ms_wlt->getScrAddrObjByKey(addrVec[0]);
   EXPECT_EQ(scrObj->getFullBalance(), 9 * COIN);
   scrObj = wlt_singleSig->getScrAddrObjByKey(asset4_singlesig->getPrefixedHash());
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsWithWalletTest, ChainZC_RBFchild_Test)
{
   //create spender lambda
   auto getSpenderPtr = [](
      const UnspentTxOut& utxo,
      shared_ptr<ResolverFeed> feed, bool flagRBF)
      ->shared_ptr<ScriptSpender>
   {
      UTXO entry(utxo.value_, utxo.txHeight_, utxo.txIndex_, utxo.txOutIndex_,
         move(utxo.txHash_), move(utxo.script_));

      auto spender = make_shared<ScriptSpender>(entry, feed);

      if (flagRBF)
         spender->setSequence(UINT32_MAX - 2);

      return spender;
   };

   BinaryData ZCHash1, ZCHash2, ZCHash3;

   //
   TestUtils::setBlocks({ "0", "1", "2", "3" }, blk0dat_);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);
   scrAddrVec.push_back(TestChain::scrAddrD);
   scrAddrVec.push_back(TestChain::scrAddrE);

   //// create assetWlt ////

   //create a root private key
   auto&& wltRoot = CryptoPRNG::generateRandom(32);
   auto assetWlt = AssetWallet_Single::createFromPrivateRoot_Armory135(
      homedir_,
      move(wltRoot), //root as a r value
      {},
      SecureBinaryData(),
      SecureBinaryData(), 
      10); //set lookup computation to 5 entries

           //register with db
   vector<BinaryData> addrVec;

   auto hashSet = assetWlt->getAddrHashSet();
   vector<BinaryData> hashVec;
   hashVec.insert(hashVec.begin(), hashSet.begin(), hashSet.end());

   DBTestUtils::registerWallet(clients_, bdvID, hashVec, assetWlt->getID());
   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");

   auto bdvPtr = DBTestUtils::getBDV(clients_, bdvID);

   //wait on signals
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);
   EXPECT_EQ(DBTestUtils::getTopBlockHeight(iface_, HEADERS), 3);

   auto wlt = bdvPtr->getWalletOrLockbox(wallet1id);
   auto dbAssetWlt = bdvPtr->getWalletOrLockbox(assetWlt->getID());

   //check balances
   const ScrAddrObj* scrObj;
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 55 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrD);
   EXPECT_EQ(scrObj->getFullBalance(), 5 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrE);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);

   //check new wallet balances
   for (auto& scripthash : hashSet)
   {
      scrObj = dbAssetWlt->getScrAddrObjByKey(scripthash);
      EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
   }

   {
      ////spend 27 from wlt to assetWlt's first 2 unused addresses
      ////send rest back to scrAddrA

      auto spendVal = 27 * COIN;
      Signer signer;

      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);

      //get utxo list for spend value
      auto&& unspentVec = wlt->getSpendableTxOutListForValue(spendVal);

      vector<UnspentTxOut> utxoVec;
      uint64_t tval = 0;
      auto utxoIter = unspentVec.begin();
      while (utxoIter != unspentVec.end())
      {
         tval += utxoIter->getValue();
         utxoVec.push_back(*utxoIter);

         if (tval > spendVal)
            break;

         ++utxoIter;
      }

      //create script spender objects
      uint64_t total = 0;
      for (auto& utxo : utxoVec)
      {
         total += utxo.getValue();
         signer.addSpender(getSpenderPtr(utxo, feed, true));
      }

      //spend 12 to first address
      auto addr0 = assetWlt->getNewAddress();
      signer.addRecipient(addr0->getRecipient(12 * COIN));
      addrVec.push_back(addr0->getPrefixedHash());

      //spend 15 to addr 1, use P2PKH
      auto addr1 = assetWlt->getNewAddress();
      signer.addRecipient(addr1->getRecipient(15 * COIN));
      addrVec.push_back(addr1->getPrefixedHash());

      if (total > spendVal)
      {
         //deal with change, no fee
         auto changeVal = total - spendVal;
         auto recipientChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), changeVal);
         signer.addRecipient(recipientChange);
      }

      //sign, verify then broadcast
      signer.sign();
      EXPECT_TRUE(signer.verify());

      auto rawTx = signer.serializeSignedTx();
      DBTestUtils::ZcVector zcVec;
      zcVec.push_back(rawTx, 14000000);

      ZCHash1 = move(BtcUtils::getHash256(rawTx));
      DBTestUtils::pushNewZc(theBDMt_, zcVec);
      DBTestUtils::waitOnNewZcSignal(clients_, bdvID);
   }

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 55 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrD);
   EXPECT_EQ(scrObj->getFullBalance(), 8 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrE);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);

   //check new wallet balances
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[0]);
   EXPECT_EQ(scrObj->getFullBalance(), 12 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[1]);
   EXPECT_EQ(scrObj->getFullBalance(), 15 * COIN);

   //grab ledger
   auto zcledger = DBTestUtils::getLedgerEntryFromWallet(dbAssetWlt, ZCHash1);
   EXPECT_EQ(zcledger.getValue(), 27 * COIN);
   //EXPECT_EQ(zcledger.getTxTime(), 14000000);
   EXPECT_TRUE(zcledger.isOptInRBF());

   //cpfp the first zc
   {
      Signer signer3;

      //instantiate resolver feed overloaded object
      auto assetFeed = make_shared<ResolverFeed_AssetWalletSingle>(assetWlt);

      //get utxo list for spend value
      auto&& unspentVec = dbAssetWlt->getSpendableTxOutListZC();

      //create script spender objects
      uint64_t total = 0;
      for (auto& utxo : unspentVec)
      {
         total += utxo.getValue();
         signer3.addSpender(getSpenderPtr(utxo, assetFeed, true));
      }

      //spend 4 to new address
      auto addr0 = assetWlt->getNewAddress();
      signer3.addRecipient(addr0->getRecipient(4 * COIN));
      addrVec.push_back(addr0->getPrefixedHash());

      //spend 6 to new address
      auto addr1 = assetWlt->getNewAddress();
      signer3.addRecipient(addr1->getRecipient(6 * COIN));
      addrVec.push_back(addr1->getPrefixedHash());

      //deal with change, no fee
      auto changeVal = total - 10 * COIN;
      auto recipientChange = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrD.getSliceCopy(1, 20), changeVal);
      signer3.addRecipient(recipientChange);

      //sign, verify then broadcast
      {
         auto lock = assetWlt->lockDecryptedContainer();
         signer3.sign();
      }

      auto rawTx = signer3.serializeSignedTx();
      DBTestUtils::ZcVector zcVec3;
      zcVec3.push_back(rawTx, 15000000);

      ZCHash2 = move(BtcUtils::getHash256(rawTx));
      DBTestUtils::pushNewZc(theBDMt_, zcVec3);
      DBTestUtils::waitOnNewZcSignal(clients_, bdvID);
   }

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 55 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrD);
   EXPECT_EQ(scrObj->getFullBalance(), 25 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrE);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);

   //check new wallet balances
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[0]);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[1]);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[2]);
   EXPECT_EQ(scrObj->getFullBalance(), 4 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[3]);
   EXPECT_EQ(scrObj->getFullBalance(), 6 * COIN);


   //grab ledgers

   //first zc should be valid still
   auto zcledger1 = DBTestUtils::getLedgerEntryFromWallet(dbAssetWlt, ZCHash1);
   EXPECT_EQ(zcledger1.getValue(), 27 * COIN);
   //EXPECT_EQ(zcledger1.getTxTime(), 14000000);
   EXPECT_TRUE(zcledger1.isOptInRBF());

   //second zc should be valid
   auto zcledger2 = DBTestUtils::getLedgerEntryFromWallet(dbAssetWlt, ZCHash2);
   EXPECT_EQ(zcledger2.getValue(), -17 * COIN);
   //EXPECT_EQ(zcledger2.getTxTime(), 15000000);
   EXPECT_TRUE(zcledger2.isOptInRBF());

   //rbf the child
   {
      auto spendVal = 10 * COIN;
      Signer signer2;

      //instantiate resolver feed
      auto assetFeed =
         make_shared<ResolverFeed_AssetWalletSingle>(assetWlt);

      //get utxo list for spend value
      auto&& unspentVec = dbAssetWlt->getRBFTxOutList();

      vector<UnspentTxOut> utxoVec;
      uint64_t tval = 0;
      auto utxoIter = unspentVec.begin();
      while (utxoIter != unspentVec.end())
      {
         tval += utxoIter->getValue();
         utxoVec.push_back(*utxoIter);

         if (tval > spendVal)
            break;

         ++utxoIter;
      }

      //create script spender objects
      uint64_t total = 0;
      for (auto& utxo : utxoVec)
      {
         total += utxo.getValue();
         signer2.addSpender(getSpenderPtr(utxo, assetFeed, true));
      }

      //spend 5 to new address
      auto addr0 = assetWlt->getNewAddress();
      signer2.addRecipient(addr0->getRecipient(6 * COIN));
      addrVec.push_back(addr0->getPrefixedHash());


      if (total > spendVal)
      {
         //change addrE, 1 btc fee
         auto changeVal = 5 * COIN;
         auto recipientChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), changeVal);
         signer2.addRecipient(recipientChange);
      }

      //sign, verify then broadcast
      {
         auto lock = assetWlt->lockDecryptedContainer();
         signer2.sign();
      }
      EXPECT_TRUE(signer2.verify());

      auto rawTx = signer2.serializeSignedTx();
      DBTestUtils::ZcVector zcVec2;
      zcVec2.push_back(rawTx, 17000000);

      ZCHash3 = move(BtcUtils::getHash256(rawTx));
      DBTestUtils::pushNewZc(theBDMt_, zcVec2);
      DBTestUtils::waitOnNewZcSignal(clients_, bdvID);
   }

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 55 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrD);
   EXPECT_EQ(scrObj->getFullBalance(), 8 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrE);
   EXPECT_EQ(scrObj->getFullBalance(), 5 * COIN);

   //check new wallet balances
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[0]);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[1]);
   EXPECT_EQ(scrObj->getFullBalance(), 15 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[2]);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[3]);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[4]);
   EXPECT_EQ(scrObj->getFullBalance(), 6 * COIN);

   //grab ledgers

   //first zc should be replaced, hence the ledger should be empty
   auto zcledger3 = DBTestUtils::getLedgerEntryFromWallet(dbAssetWlt, ZCHash1);
   EXPECT_EQ(zcledger3.getValue(), 27 * COIN);
   EXPECT_EQ(zcledger3.getBlockNum(), UINT32_MAX);
   EXPECT_TRUE(zcledger3.isOptInRBF());

   //second zc should be replaced
   auto zcledger8 = DBTestUtils::getLedgerEntryFromWallet(dbAssetWlt, ZCHash2);
   EXPECT_EQ(zcledger8.getTxHash(), BtcUtils::EmptyHash_);

   //third zc should be valid
   auto zcledger9 = DBTestUtils::getLedgerEntryFromWallet(dbAssetWlt, ZCHash3);
   EXPECT_EQ(zcledger9.getValue(), -6 * COIN);
   EXPECT_EQ(zcledger9.getBlockNum(), UINT32_MAX);
   EXPECT_TRUE(zcledger9.isOptInRBF());

   //mine a new block
   DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrA, 3);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   //check chain is 3 block longer
   EXPECT_EQ(DBTestUtils::getTopBlockHeight(iface_, HEADERS), 6);

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 200 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 30 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 55 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrD);
   EXPECT_EQ(scrObj->getFullBalance(), 8 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrE);
   EXPECT_EQ(scrObj->getFullBalance(), 5 * COIN);

   //check new wallet balances
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[0]);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[1]);
   EXPECT_EQ(scrObj->getFullBalance(), 15 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[2]);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[3]);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
   scrObj = dbAssetWlt->getScrAddrObjByKey(addrVec[4]);
   EXPECT_EQ(scrObj->getFullBalance(), 6 * COIN);

   //check all zc are mined with 1 conf
   zcledger3 = DBTestUtils::getLedgerEntryFromWallet(dbAssetWlt, ZCHash1);
   EXPECT_EQ(zcledger3.getValue(), 27 * COIN);
   EXPECT_EQ(zcledger3.getBlockNum(), 4);
   EXPECT_FALSE(zcledger3.isOptInRBF());

   zcledger9 = DBTestUtils::getLedgerEntryFromWallet(dbAssetWlt, ZCHash3);
   EXPECT_EQ(zcledger9.getValue(), -6 * COIN);
   EXPECT_EQ(zcledger9.getBlockNum(), 4);
   EXPECT_FALSE(zcledger9.isOptInRBF());
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsWithWalletTest, ZC_InOut_SameBlock)
{
   //create spender lambda
   auto getSpenderPtr = [](
      const UnspentTxOut& utxo,
      shared_ptr<ResolverFeed> feed, bool flagRBF)
      ->shared_ptr<ScriptSpender>
   {
      UTXO entry(utxo.value_, utxo.txHeight_, utxo.txIndex_, utxo.txOutIndex_,
         move(utxo.txHash_), move(utxo.script_));

      auto spender = make_shared<ScriptSpender>(entry, feed);

      if (flagRBF)
         spender->setSequence(UINT32_MAX - 2);

      return spender;
   };

   BinaryData ZCHash1, ZCHash2, ZCHash3;

   //
   TestUtils::setBlocks({ "0", "1" }, blk0dat_);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);

   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");

   auto bdvPtr = DBTestUtils::getBDV(clients_, bdvID);

   //wait on signals
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);
   auto wlt = bdvPtr->getWalletOrLockbox(wallet1id);

   //check balances
   const ScrAddrObj* scrObj;
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);

   //add the 2 zc
   auto&& ZC1 = TestUtils::getTx(2, 1); //block 2, tx 1
   auto&& ZChash1 = BtcUtils::getHash256(ZC1);

   auto&& ZC2 = TestUtils::getTx(2, 2); //block 2, tx 2
   auto&& ZChash2 = BtcUtils::getHash256(ZC2);

   DBTestUtils::ZcVector rawZcVec;
   rawZcVec.push_back(ZC1, 1300000000);
   rawZcVec.push_back(ZC2, 1310000000);

   DBTestUtils::pushNewZc(theBDMt_, rawZcVec);
   DBTestUtils::waitOnNewZcSignal(clients_, bdvID);

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 5 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);

   //add last block
   TestUtils::appendBlocks({ "2" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   //check balances
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrA);
   EXPECT_EQ(scrObj->getFullBalance(), 50 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrB);
   EXPECT_EQ(scrObj->getFullBalance(), 55 * COIN);
   scrObj = wlt->getScrAddrObjByKey(TestChain::scrAddrC);
   EXPECT_EQ(scrObj->getFullBalance(), 0 * COIN);
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(BlockUtilsWithWalletTest, ZC_MineAfter1Block)
{
   auto feed = make_shared<ResolverUtils::TestResolverFeed>();

   auto addToFeed = [feed](const BinaryData& key)->void
   {
      auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
      feed->h160ToPubKey_.insert(datapair);
      feed->pubKeyToPrivKey_[datapair.second] = key;
   };

   addToFeed(TestChain::privKeyAddrB);
   addToFeed(TestChain::privKeyAddrC);
   addToFeed(TestChain::privKeyAddrD);

   ////
   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);
   scrAddrVec.push_back(TestChain::scrAddrD);

   theBDMt_->start(config.initMode_);
   auto&& bdvID = DBTestUtils::registerBDV(clients_, NetworkConfig::getMagicBytes());

   DBTestUtils::registerWallet(clients_, bdvID, scrAddrVec, "wallet1");

   auto bdvPtr = DBTestUtils::getBDV(clients_, bdvID);

   //wait on signals
   DBTestUtils::goOnline(clients_, bdvID);
   DBTestUtils::waitOnBDMReady(clients_, bdvID);
   auto wlt = bdvPtr->getWalletOrLockbox(wallet1id);

   uint64_t balanceWlt;

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrA)->getFullBalance();
   EXPECT_EQ(balanceWlt, 50 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrB)->getFullBalance();
   EXPECT_EQ(balanceWlt, 70 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrC)->getFullBalance();
   EXPECT_EQ(balanceWlt, 20 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrD)->getFullBalance();
   EXPECT_EQ(balanceWlt, 65 * COIN);

   //spend from B to C
   auto&& utxoVec = wlt->getSpendableTxOutListForValue();

   UTXO utxoA, utxoB;
   for (auto& utxo : utxoVec)
   {
      if (utxo.getRecipientScrAddr() == TestChain::scrAddrD)
      {
         utxoA.value_ = utxo.value_;
         utxoA.script_ = utxo.script_;
         utxoA.txHeight_ = utxo.txHeight_;
         utxoA.txIndex_ = utxo.txIndex_;
         utxoA.txOutIndex_ = utxo.txOutIndex_;
         utxoA.txHash_ = utxo.txHash_;
      }
      else if (utxo.getRecipientScrAddr() == TestChain::scrAddrB)
      {
         utxoB.value_ = utxo.value_;
         utxoB.script_ = utxo.script_;
         utxoB.txHeight_ = utxo.txHeight_;
         utxoB.txIndex_ = utxo.txIndex_;
         utxoB.txOutIndex_ = utxo.txOutIndex_;
         utxoB.txHash_ = utxo.txHash_;
      }
   }

   auto spenderA = make_shared<ScriptSpender>(utxoA);
   auto spenderB = make_shared<ScriptSpender>(utxoB);

   DBTestUtils::ZcVector zcVec;

   //spend from D to C
   {
      Signer signer;
      signer.addSpender(spenderA);

      auto recipient = std::make_shared<Recipient_P2PKH>(
         TestChain::scrAddrC.getSliceCopy(1, 20), utxoA.getValue());
      signer.addRecipient(recipient);

      signer.setFeed(feed);
      signer.sign();
      auto rawTx = signer.serializeSignedTx();
      zcVec.push_back(signer.serializeSignedTx(), 130000000, 0);
   }
   
   //spend from B to C
   {
      Signer signer;
      signer.addSpender(spenderB);

      auto recipient = std::make_shared<Recipient_P2PKH>(
         TestChain::scrAddrC.getSliceCopy(1, 20), utxoB.getValue());
      signer.addRecipient(recipient);

      signer.setFeed(feed);
      signer.sign();
      zcVec.push_back(signer.serializeSignedTx(), 131000000, 1);
   }

   auto hash1 = zcVec.zcVec_[0].first.getThisHash();
   auto hash2 = zcVec.zcVec_[1].first.getThisHash();

   //broadcast
   DBTestUtils::pushNewZc(theBDMt_, zcVec);
   DBTestUtils::waitOnNewZcSignal(clients_, bdvID);

   //check balances
   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrA)->getFullBalance();
   EXPECT_EQ(balanceWlt, 50 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrB)->getFullBalance();
   EXPECT_EQ(balanceWlt, 50 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrC)->getFullBalance();
   EXPECT_EQ(balanceWlt, 45 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrD)->getFullBalance();
   EXPECT_EQ(balanceWlt, 60 * COIN);

   auto zc1 = bdvPtr->getTxByHash(hash1);
   auto zc2 = bdvPtr->getTxByHash(hash2);

   EXPECT_EQ(zc1.getTxHeight(), UINT32_MAX);
   EXPECT_EQ(zc2.getTxHeight(), UINT32_MAX);

   //mine 1 block
   DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrA, 1);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   //check balances
   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrA)->getFullBalance();
   EXPECT_EQ(balanceWlt, 100 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrB)->getFullBalance();
   EXPECT_EQ(balanceWlt, 50 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrC)->getFullBalance();
   EXPECT_EQ(balanceWlt, 45 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrD)->getFullBalance();
   EXPECT_EQ(balanceWlt, 60 * COIN);

   zc1 = bdvPtr->getTxByHash(hash1);
   zc2 = bdvPtr->getTxByHash(hash2);

   EXPECT_EQ(zc1.getTxHeight(), 6);
   EXPECT_EQ(zc2.getTxHeight(), UINT32_MAX);

   //mine last block
   DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrB, 1);
   DBTestUtils::waitOnNewBlockSignal(clients_, bdvID);

   //check balances
   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrA)->getFullBalance();
   EXPECT_EQ(balanceWlt, 100 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrB)->getFullBalance();
   EXPECT_EQ(balanceWlt, 100 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrC)->getFullBalance();
   EXPECT_EQ(balanceWlt, 45 * COIN);

   balanceWlt = wlt->getScrAddrObjByKey(TestChain::scrAddrD)->getFullBalance();
   EXPECT_EQ(balanceWlt, 60 * COIN);

   zc1 = bdvPtr->getTxByHash(hash1);
   zc2 = bdvPtr->getTxByHash(hash2);

   EXPECT_EQ(zc1.getTxHeight(), 6);
   EXPECT_EQ(zc2.getTxHeight(), 7);
}

////////////////////////////////////////////////////////////////////////////////
class WebSocketTests : public ::testing::Test
{
protected:
   BlockDataManagerThread *theBDMt_;
   PassphraseLambda authPeersPassLbd_;

   void initBDM(void)
   {
      auto& magicBytes = NetworkConfig::getMagicBytes();
        
      nodePtr_ = make_shared<NodeUnitTest>(
         *(uint32_t*)magicBytes.getPtr(), false);
      auto watcherPtr = make_shared<NodeUnitTest>(
         *(uint32_t*)magicBytes.getPtr(), true);

      rpcNode_ =  make_shared<NodeRPC_UnitTest>(
         nodePtr_, watcherPtr);

      config.bitcoinNodes_ = make_pair(nodePtr_, watcherPtr);
      config.rpcNode_ = rpcNode_;

      //randomized peer keys, in ram only
      config.ephemeralPeers_ = true;

      theBDMt_ = new BlockDataManagerThread(config);
      iface_ = theBDMt_->bdm()->getIFace();
      nodePtr_->setIface(iface_);

      nodePtr_->setBlockchain(theBDMt_->bdm()->blockchain());
      nodePtr_->setBlockFiles(theBDMt_->bdm()->blockFiles());
   }

   /////////////////////////////////////////////////////////////////////////////
   virtual void SetUp()
   {
      LOGDISABLESTDOUT();
      zeros_ = READHEX("00000000");

      blkdir_ = string("./blkfiletest");
      homedir_ = string("./fakehomedir");
      ldbdir_ = string("./ldbtestdir");

      DBUtils::removeDirectory(blkdir_);
      DBUtils::removeDirectory(homedir_);
      DBUtils::removeDirectory(ldbdir_);

      mkdir(blkdir_);
      mkdir(homedir_);
      mkdir(ldbdir_);

      // Put the first 5 blocks into the blkdir
      BlockDataManagerConfig::setServiceType(SERVICE_WEBSOCKET);
      BlockDataManagerConfig::setOperationMode(OPERATION_UNITTEST);

      blk0dat_ = BtcUtils::getBlkFilename(blkdir_, 0);
      TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);

      BlockDataManagerConfig::setDbType(ARMORY_DB_SUPER);
      config.blkFileLocation_ = blkdir_;
      config.dbDir_ = ldbdir_;
      config.threadCount_ = 3;
      config.dataDir_ = homedir_;
      config.ephemeralPeers_ = false;

      unsigned port_int = 51152;
      stringstream port_ss;
      port_ss << port_int;
      config.listenPort_ = port_ss.str();

      startupBIP151CTX();
      startupBIP150CTX(4, false);

      //setup auth peers for server and client
      authPeersPassLbd_ = [](const set<BinaryData>&)->SecureBinaryData
      {
         return SecureBinaryData::fromString("authpeerpass");
      };

      AuthorizedPeers serverPeers(
         homedir_, SERVER_AUTH_PEER_FILENAME, authPeersPassLbd_);
      AuthorizedPeers clientPeers(
         homedir_, CLIENT_AUTH_PEER_FILENAME, authPeersPassLbd_);

      //share public keys between client and server
      auto& serverPubkey = serverPeers.getOwnPublicKey();
      auto& clientPubkey = clientPeers.getOwnPublicKey();

      stringstream serverAddr;
      serverAddr << "127.0.0.1:" << config.listenPort_;
      clientPeers.addPeer(serverPubkey, serverAddr.str());
      serverPeers.addPeer(clientPubkey, "127.0.0.1");

      wallet1id = "wallet1";

      initBDM();
   }

   /////////////////////////////////////////////////////////////////////////////
   virtual void TearDown(void)
   {
      shutdownBIP151CTX();
      
      delete theBDMt_;
      theBDMt_ = nullptr;

      DBUtils::removeDirectory(blkdir_);
      DBUtils::removeDirectory(homedir_);
      DBUtils::removeDirectory("./ldbtestdir");

      mkdir("./ldbtestdir");

      LOGENABLESTDOUT();
      CLEANUP_ALL_TIMERS();
   }

   BlockDataManagerConfig config;

   LMDBBlockDatabase* iface_;
   BinaryData zeros_;

   string blkdir_;
   string homedir_;
   string ldbdir_;
   string blk0dat_;

   string wallet1id;

   shared_ptr<NodeUnitTest> nodePtr_;
   shared_ptr<NodeRPC_UnitTest> rpcNode_;
};

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_ParallelAsync)
{
   //public server
   startupBIP150CTX(4, true);

   //
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   auto&& firstHash = READHEX("b6b6f145742a9072fd85f96772e63a00eb4101709aa34ec5dd59e8fc904191a7");

   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   auto createNAddresses = [](unsigned count)->vector<BinaryData>
   {
      vector<BinaryData> result;

      for (unsigned i = 0; i < count; i++)
      {
         BinaryWriter bw;
         bw.put_uint8_t(SCRIPT_PREFIX_HASH160);

         auto&& addrData = CryptoPRNG::generateRandom(20);
         bw.put_BinaryData(addrData);

         result.push_back(bw.getData());
      }

      return result;
   };

   auto&& _scrAddrVec = createNAddresses(2000);
   _scrAddrVec.push_back(TestChain::scrAddrA);
   _scrAddrVec.push_back(TestChain::scrAddrB);
   _scrAddrVec.push_back(TestChain::scrAddrC);
   _scrAddrVec.push_back(TestChain::scrAddrE);

   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_,  BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_,pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());
      
      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      auto delegate = move(DBTestUtils::getLedgerDelegate(bdvObj));
      auto ledgers = move(DBTestUtils::getHistoryPage(delegate, 0));

      bdvObj->unregisterFromDB();
   }

   unsigned nThreads = 50;
   vector<shared_ptr<atomic<unsigned>>> times(nThreads);
   for (unsigned z=0; z<nThreads; z++)
      times[z] = make_shared<atomic<unsigned>>();
   atomic<unsigned> counter = {0};
   auto request_lambda = [&](void)->void
   {
      auto this_id = counter.fetch_add(1, memory_order_relaxed);
      auto rightnow = chrono::system_clock::now();
      auto&& scrAddrVec = createNAddresses(6);
      scrAddrVec.push_back(TestChain::scrAddrA);
      scrAddrVec.push_back(TestChain::scrAddrB);
      scrAddrVec.push_back(TestChain::scrAddrC);
      scrAddrVec.push_back(TestChain::scrAddrE);

      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_,  BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_,pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      const vector<BinaryData> lb1ScrAddrs
      {
         TestChain::lb1ScrAddr,
         TestChain::lb1ScrAddrP2SH
      };
      const vector<BinaryData> lb2ScrAddrs
      {
         TestChain::lb2ScrAddr,
         TestChain::lb2ScrAddrP2SH
      };

      vector<string> walletRegIDs;

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
      walletRegIDs.push_back(
         wallet1.registerAddresses(scrAddrVec, false));

      scrAddrVec.push_back(TestChain::scrAddrD);
      auto&& wallet2 = bdvObj->instantiateWallet("wallet2");
      walletRegIDs.push_back(
         wallet2.registerAddresses(scrAddrVec, false));

      auto&& lb1 = bdvObj->instantiateLockbox("lb1");
      walletRegIDs.push_back(
         lb1.registerAddresses(lb1ScrAddrs, false));

      auto&& lb2 = bdvObj->instantiateLockbox("lb2");
      walletRegIDs.push_back(
         lb2.registerAddresses(lb2ScrAddrs, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);


      //get wallets delegate
      auto del1_prom = make_shared<promise<AsyncClient::LedgerDelegate>>();
      auto del1_fut = del1_prom->get_future();
      auto del1_get = [del1_prom](ReturnMessage<AsyncClient::LedgerDelegate> delegate)->void
      {
         del1_prom->set_value(move(delegate.get()));
      };
      bdvObj->getLedgerDelegateForWallets(del1_get);

      vector<AsyncClient::LedgerDelegate> delV(21);

      auto getAddrDelegate = [bdvObj](const BinaryData& scrAddr, 
         string walletId, AsyncClient::LedgerDelegate* delPtr)->void
      {
         //get scrAddr delegates
         auto del_prom = make_shared<promise<AsyncClient::LedgerDelegate>>();
         auto del_fut = del_prom->get_future();
         auto del_get = [del_prom](ReturnMessage<AsyncClient::LedgerDelegate> delegate)->void
         {
            del_prom->set_value(move(delegate.get()));
         };
         bdvObj->getLedgerDelegateForScrAddr(
            walletId, scrAddr, del_get);
         *delPtr = move(del_fut.get());
      };
      
      auto delegate = move(del1_fut.get());

      deque<thread> delThr;
      for (unsigned i = 0; i < 10; i++)
      {
         delThr.push_back(
            thread(getAddrDelegate, scrAddrVec[i], "wallet1", &delV[i]));
      }

      for (unsigned i = 10; i < 21; i++)
      {
         delThr.push_back(
            thread(getAddrDelegate, scrAddrVec[i - 10], "wallet2", &delV[i]));
      }

      for (auto& thr : delThr)
      {
         if (thr.joinable())
            thr.join();
      }

      //get ledgers
      auto ledger_prom = 
         make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
      auto ledger_fut = ledger_prom->get_future();
      auto ledger_get = 
         [ledger_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
      {
         ledger_prom->set_value(move(ledgerV.get()));
      };
      delegate.getHistoryPage(0, ledger_get);

      //get addr ledgers
      deque<vector<::ClientClasses::LedgerEntry>> addrLedgerV(21);
      auto getAddrLedger = [bdvObj](
         AsyncClient::LedgerDelegate delegate, 
         vector<::ClientClasses::LedgerEntry>* addrLedger)->void
      {
         auto ledger_prom = 
            make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
         auto ledger_fut = ledger_prom->get_future();
         auto ledger_get = 
            [ledger_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
         {
            ledger_prom->set_value(move(ledgerV.get()));
         };

         delegate.getHistoryPage(0, ledger_get);
         *addrLedger = move(ledger_fut.get());
      };

      delThr.clear();

      for (unsigned i = 0; i < 21; i++)
         delThr.push_back(thread(getAddrLedger, delV[i], &addrLedgerV[i]));

      //
      auto w1AddrBal_prom = make_shared<promise<map<BinaryData, vector<uint64_t>>>>();
      auto w1AddrBal_fut = w1AddrBal_prom->get_future();
      auto w1_getAddrBalancesLBD = 
         [w1AddrBal_prom](ReturnMessage<map<BinaryData, vector<uint64_t>>> balances)->void
      {
         w1AddrBal_prom->set_value(move(balances.get()));
      };
      wallet1.getAddrBalancesFromDB(w1_getAddrBalancesLBD);
      
      //
      auto w1Bal_prom = make_shared<promise<vector<uint64_t>>>();
      auto w1Bal_fut = w1Bal_prom->get_future();
      auto w1_getBalanceAndCountLBD = 
         [w1Bal_prom](ReturnMessage<vector<uint64_t>> balances)->void
      {
         w1Bal_prom->set_value(move(balances.get()));
      };
      wallet1.getBalancesAndCount(5, w1_getBalanceAndCountLBD);

      //
      auto lb1AddrBal_prom = make_shared<promise<map<BinaryData, vector<uint64_t>>>>();
      auto lb1AddrBal_fut = lb1AddrBal_prom->get_future();
      auto lb1_getAddrBalancesLBD = 
         [lb1AddrBal_prom](ReturnMessage<map<BinaryData, vector<uint64_t>>> balances)->void
      {
         lb1AddrBal_prom->set_value(move(balances.get()));
      };
      lb1.getAddrBalancesFromDB(lb1_getAddrBalancesLBD);

      //
      auto lb2AddrBal_prom = make_shared<promise<map<BinaryData, vector<uint64_t>>>>();
      auto lb2AddrBal_fut = lb2AddrBal_prom->get_future();
      auto lb2_getAddrBalancesLBD = 
         [lb2AddrBal_prom](ReturnMessage<map<BinaryData, vector<uint64_t>>> balances)->void
      {
         lb2AddrBal_prom->set_value(move(balances.get()));
      };
      lb2.getAddrBalancesFromDB(lb2_getAddrBalancesLBD);

      //
      auto lb1Bal_prom = make_shared<promise<vector<uint64_t>>>();
      auto lb1Bal_fut = lb1Bal_prom->get_future();
      auto lb1_getBalanceAndCountLBD = 
         [lb1Bal_prom](ReturnMessage<vector<uint64_t>> balances)->void
      {
         lb1Bal_prom->set_value(move(balances.get()));
      };
      lb1.getBalancesAndCount(5, lb1_getBalanceAndCountLBD);

      //
      auto lb2Bal_prom = make_shared<promise<vector<uint64_t>>>();
      auto lb2Bal_fut = lb2Bal_prom->get_future();
      auto lb2_getBalanceAndCountLBD = 
         [lb2Bal_prom](ReturnMessage<vector<uint64_t>> balances)->void
      {
         lb2Bal_prom->set_value(move(balances.get()));
      };
      lb2.getBalancesAndCount(5, lb2_getBalanceAndCountLBD);

      //get tx
      auto tx_prom = make_shared<promise<AsyncClient::TxResult>>();
      auto tx_fut = tx_prom->get_future();
      auto tx_get = [tx_prom](ReturnMessage<AsyncClient::TxResult> tx)->void
      {
         tx_prom->set_value(move(tx.get()));
      };
      bdvObj->getTxByHash(firstHash, tx_get);

      //get utxos
      auto utxo_prom = make_shared<promise<vector<UTXO>>>();
      auto utxo_fut = utxo_prom->get_future();
      auto utxo_get = [utxo_prom](ReturnMessage<vector<UTXO>> utxoV)->void
      {
         utxo_prom->set_value(move(utxoV.get()));
      };
      wallet1.getSpendableTxOutListForValue(UINT64_MAX, utxo_get);

      //wait on futures
      auto w1AddrBalances = move(w1AddrBal_fut.get());
      auto w1Balances = move(w1Bal_fut.get());
      auto lb1AddrBalances = move(lb1AddrBal_fut.get());
      auto lb2AddrBalances = move(lb2AddrBal_fut.get());
      auto lb1Balances = move(lb1Bal_fut.get());
      auto lb2Balances = move(lb2Bal_fut.get());
      auto ledgers = move(ledger_fut.get());
      auto tx = move(tx_fut.get());
      auto utxos = move(utxo_fut.get());

      //w1 addr balances
      auto balanceVec = w1AddrBalances[TestChain::scrAddrA];
      EXPECT_EQ(balanceVec[0], 50 * COIN);
      balanceVec = w1AddrBalances[TestChain::scrAddrB];
      EXPECT_EQ(balanceVec[0], 70 * COIN);
      balanceVec = w1AddrBalances[TestChain::scrAddrC];
      EXPECT_EQ(balanceVec[0], 20 * COIN);

      //w1 balances
      auto fullBalance = w1Balances[0];
      auto spendableBalance = w1Balances[1];
      auto unconfirmedBalance = w1Balances[2];
      EXPECT_EQ(fullBalance, 170 * COIN);
      EXPECT_EQ(spendableBalance, 70 * COIN);
      EXPECT_EQ(unconfirmedBalance, 170 * COIN);

      //lb1 addr balances
      balanceVec = lb1AddrBalances[TestChain::lb1ScrAddr];
      EXPECT_EQ(balanceVec[0], 5 * COIN);
      balanceVec = lb1AddrBalances[TestChain::lb1ScrAddrP2SH];
      EXPECT_EQ(balanceVec[0], 25 * COIN);

      //lb2 addr balances
      balanceVec = lb2AddrBalances[TestChain::lb2ScrAddr];
      EXPECT_EQ(balanceVec[0], 30 * COIN);
      balanceVec = lb2AddrBalances[TestChain::lb2ScrAddrP2SH];
      EXPECT_EQ(balanceVec.size(), 0);

      //lb1 balances
      EXPECT_EQ(lb1Balances[0], 30 * COIN);

      //lb2 balances
      EXPECT_EQ(lb2Balances[0], 30 * COIN);

      //grab main ledgers
      auto& firstEntry = ledgers[1];
      auto txHash = firstEntry.getTxHash();
      EXPECT_EQ(firstHash, txHash);

      //check first tx
      EXPECT_EQ(tx->getThisHash(), firstHash);

      //check utxos
      EXPECT_EQ(utxos.size(), 5);

      //grab all tx for each utxo
      map<BinaryData, shared_future<AsyncClient::TxResult>> futMap;
      for(auto& utxo : utxos)
      {
         auto& hash = utxo.getTxHash();
         if (futMap.find(hash) != futMap.end())
            continue;

         auto utxoProm = make_shared<promise<AsyncClient::TxResult>>();
         futMap.insert(make_pair(hash, utxoProm->get_future()));
         auto utxoLBD = [utxoProm](ReturnMessage<AsyncClient::TxResult> tx)->void
         {
            utxoProm->set_value(move(tx.get()));
         };
         bdvObj->getTxByHash(hash, utxoLBD);
      }

      for(auto& fut_pair : futMap)
      {
         auto txobj = move(fut_pair.second.get());
         EXPECT_EQ(txobj->getThisHash(), fut_pair.first);
      }

      for (auto& thr : delThr)
      {
         if (thr.joinable())
            thr.join();
      }

      for (unsigned i = 0; i < 6; i++)
         EXPECT_EQ(addrLedgerV[i].size(), 0);
      EXPECT_EQ(addrLedgerV[6].size(), 1);
      EXPECT_EQ(addrLedgerV[7].size(), 7);
      EXPECT_EQ(addrLedgerV[8].size(), 4);
      EXPECT_EQ(addrLedgerV[9].size(), 2);
      EXPECT_EQ(addrLedgerV[20].size(), 4);

      for (unsigned i = 0; i < 10; i++)
      {
         auto& v1 = addrLedgerV[i];         
         auto& v2 = addrLedgerV[i + 10];

         if (v1.size() != v2.size())
            EXPECT_TRUE(false);

         for (unsigned y = 0; y < v1.size(); y++)
         {
            if(!(v1[y] == v2[y]))
               EXPECT_TRUE(false);
         }
      }

      auto rekeyCount = bdvObj->getRekeyCount();
      EXPECT_EQ(rekeyCount.first, 2);
      EXPECT_TRUE(rekeyCount.second > 7);
      bdvObj->unregisterFromDB();

      auto time_ms = chrono::duration_cast<chrono::milliseconds>(
         chrono::system_clock::now() - rightnow);
      times[this_id]->store(time_ms.count(), memory_order_relaxed);
   };

   vector<thread> thrV;
   for(unsigned ct=0; ct<nThreads; ct++)
      thrV.push_back(thread(request_lambda));

   for(auto& thr : thrV)
   {
      if(thr.joinable())
         thr.join();
   }

   {
      struct comparator
      {
         inline bool operator()(const shared_ptr<atomic<unsigned>>& lhs, const shared_ptr<atomic<unsigned>>& rhs)
         {
            return lhs->load(memory_order_relaxed) < rhs->load(memory_order_relaxed);
         }
      };

      sort(times.begin(), times.end(), comparator());
      unsigned total = 0;
      for (auto& tp : times)
         total += tp->load(memory_order_relaxed);
      
      cout << "completion average: " << total / nThreads << endl;
      cout << "top 5:" << endl;
      for (unsigned i=nThreads -1 ; i>nThreads - 6; i--)
         cout << "  " << *times[i] << endl;

      cout << "bottom 5:" << endl;
      for (unsigned i=0; i<5; i++)
         cout << "  " << *times[i] << endl;
   }

   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, DISABLED_WebSocketStack_ParallelAsync_ShutdownClients)
{
   /***
   Create a lot of client connections in parallel and slam the db with requests,
   then shutdown some of the clients before the requests are met.
   ***/
   //public server
   startupBIP150CTX(4, true);

   //
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   auto&& firstHash = READHEX("b6b6f145742a9072fd85f96772e63a00eb4101709aa34ec5dd59e8fc904191a7");

   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   auto createNAddresses = [](unsigned count)->vector<BinaryData>
   {
      vector<BinaryData> result;

      for (unsigned i = 0; i < count; i++)
      {
         BinaryWriter bw;
         bw.put_uint8_t(SCRIPT_PREFIX_HASH160);

         auto&& addrData = CryptoPRNG::generateRandom(20);
         bw.put_BinaryData(addrData);

         result.push_back(bw.getData());
      }

      return result;
   };

   auto&& _scrAddrVec = createNAddresses(2000);
   _scrAddrVec.push_back(TestChain::scrAddrA);
   _scrAddrVec.push_back(TestChain::scrAddrB);
   _scrAddrVec.push_back(TestChain::scrAddrC);
   _scrAddrVec.push_back(TestChain::scrAddrE);

   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_,  BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_,pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());
      
      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      auto delegate = move(DBTestUtils::getLedgerDelegate(bdvObj));
      auto ledgers = move(DBTestUtils::getHistoryPage(delegate, 0));

      bdvObj->unregisterFromDB();
   }

   unsigned nThreads = 3;
   atomic<unsigned> counter = {0};
   auto request_lambda = [&](void)->void
   {
      unsigned killCount = 0;
      auto this_id = counter.fetch_add(1, memory_order_relaxed);
      auto checkForTermination = [this_id, &killCount](void)->bool
      {
         if (this_id % 3 != 0)
            return false;

         auto rndVal = CryptoPRNG::generateRandom(1);
         ++killCount;
         return rndVal.getPtr()[0] % 3 || killCount == 3;
      };

      /*
      kill 1/3rd of threads at different spots
      */ 
      auto&& scrAddrVec = createNAddresses(6);
      scrAddrVec.push_back(TestChain::scrAddrA);
      scrAddrVec.push_back(TestChain::scrAddrB);
      scrAddrVec.push_back(TestChain::scrAddrC);
      scrAddrVec.push_back(TestChain::scrAddrE);

      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_,  BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_,pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      const vector<BinaryData> lb1ScrAddrs
      {
         TestChain::lb1ScrAddr,
         TestChain::lb1ScrAddrP2SH
      };
      const vector<BinaryData> lb2ScrAddrs
      {
         TestChain::lb2ScrAddr,
         TestChain::lb2ScrAddrP2SH
      };

      vector<string> walletRegIDs;

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
      walletRegIDs.push_back(
         wallet1.registerAddresses(scrAddrVec, false));

      scrAddrVec.push_back(TestChain::scrAddrD);
      auto&& wallet2 = bdvObj->instantiateWallet("wallet2");
      walletRegIDs.push_back(
         wallet2.registerAddresses(scrAddrVec, false));

      auto&& lb1 = bdvObj->instantiateLockbox("lb1");
      walletRegIDs.push_back(
         lb1.registerAddresses(lb1ScrAddrs, false));

      auto&& lb2 = bdvObj->instantiateLockbox("lb2");
      walletRegIDs.push_back(
         lb2.registerAddresses(lb2ScrAddrs, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //get wallets delegate
      auto del1_prom = make_shared<promise<AsyncClient::LedgerDelegate>>();
      auto del1_fut = del1_prom->get_future();
      auto del1_get = [del1_prom](ReturnMessage<AsyncClient::LedgerDelegate> delegate)->void
      {
         del1_prom->set_value(move(delegate.get()));
      };
      bdvObj->getLedgerDelegateForWallets(del1_get);

      vector<shared_ptr<AsyncClient::LedgerDelegate>> delV(2);
      for (auto& delPtr : delV)
         delPtr = make_shared<AsyncClient::LedgerDelegate>();

      auto getAddrDelegate = [bdvObj](const BinaryData& scrAddr, 
         string walletId, shared_ptr<AsyncClient::LedgerDelegate> delPtr)->void
      {
         //get scrAddr delegates
         auto del_prom = make_shared<promise<AsyncClient::LedgerDelegate>>();
         auto del_fut = del_prom->get_future();
         auto del_get = [del_prom](ReturnMessage<AsyncClient::LedgerDelegate> delegate)->void
         {
            del_prom->set_value(move(delegate.get()));
         };
         bdvObj->getLedgerDelegateForScrAddr(
            walletId, scrAddr, del_get);
         *delPtr = del_fut.get();
      };
      
      auto delegate = move(del1_fut.get());

      deque<thread> delThr;
      for (unsigned i = 0; i < 1; i++)
      {
         delThr.push_back(
            thread(getAddrDelegate, scrAddrVec[i], "wallet1", delV[i]));
      }

      for (unsigned i = 1; i < 2; i++)
      {
         delThr.push_back(
            thread(getAddrDelegate, scrAddrVec[i], "wallet2", delV[i]));
      }

      //first termination spot
      if (checkForTermination())
      {
         cout << "out at first spot" << endl;
         return;
      }

      for (auto& thr : delThr)
      {
         if (thr.joinable())
            thr.join();
      }

      /*
      //get ledgers
      auto ledger_prom = 
         make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
      auto ledger_fut = ledger_prom->get_future();
      auto ledger_get = 
         [ledger_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
      {
         ledger_prom->set_value(move(ledgerV.get()));
      };
      delegate.getHistoryPage(0, ledger_get);

      //get addr ledgers
      deque<shared_ptr<vector<::ClientClasses::LedgerEntry>>> addrLedgerV(21);
      for (auto& addrLedgers : addrLedgerV)
         addrLedgers = make_shared<vector<::ClientClasses::LedgerEntry>>();

      auto getAddrLedger = [bdvObj](
         shared_ptr<AsyncClient::LedgerDelegate> delegate, 
         shared_ptr<vector<::ClientClasses::LedgerEntry>> addrLedger)->void
      {
         auto ledger_prom = 
            make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
         auto ledger_fut = ledger_prom->get_future();
         auto ledger_get = 
            [ledger_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
         {
            ledger_prom->set_value(move(ledgerV.get()));
         };

         delegate->getHistoryPage(0, ledger_get);
         *addrLedger = move(ledger_fut.get());
      };

      delThr.clear();

      for (unsigned i = 0; i < 21; i++)
         delThr.push_back(thread(getAddrLedger, delV[i], addrLedgerV[i]));


      //second termination spot
      if (checkForTermination())
      {
         cout << "out at second spot" << endl;
         return;
      }

      //
      auto w1AddrBal_prom = make_shared<promise<map<BinaryData, vector<uint64_t>>>>();
      auto w1AddrBal_fut = w1AddrBal_prom->get_future();
      auto w1_getAddrBalancesLBD = 
         [w1AddrBal_prom](ReturnMessage<map<BinaryData, vector<uint64_t>>> balances)->void
      {
         w1AddrBal_prom->set_value(move(balances.get()));
      };
      wallet1.getAddrBalancesFromDB(w1_getAddrBalancesLBD);
      
      //
      auto w1Bal_prom = make_shared<promise<vector<uint64_t>>>();
      auto w1Bal_fut = w1Bal_prom->get_future();
      auto w1_getBalanceAndCountLBD = 
         [w1Bal_prom](ReturnMessage<vector<uint64_t>> balances)->void
      {
         w1Bal_prom->set_value(move(balances.get()));
      };
      wallet1.getBalancesAndCount(5, w1_getBalanceAndCountLBD);

      //
      auto lb1AddrBal_prom = make_shared<promise<map<BinaryData, vector<uint64_t>>>>();
      auto lb1AddrBal_fut = lb1AddrBal_prom->get_future();
      auto lb1_getAddrBalancesLBD = 
         [lb1AddrBal_prom](ReturnMessage<map<BinaryData, vector<uint64_t>>> balances)->void
      {
         lb1AddrBal_prom->set_value(move(balances.get()));
      };
      lb1.getAddrBalancesFromDB(lb1_getAddrBalancesLBD);

      //
      auto lb2AddrBal_prom = make_shared<promise<map<BinaryData, vector<uint64_t>>>>();
      auto lb2AddrBal_fut = lb2AddrBal_prom->get_future();
      auto lb2_getAddrBalancesLBD = 
         [lb2AddrBal_prom](ReturnMessage<map<BinaryData, vector<uint64_t>>> balances)->void
      {
         lb2AddrBal_prom->set_value(move(balances.get()));
      };
      lb2.getAddrBalancesFromDB(lb2_getAddrBalancesLBD);

      //
      auto lb1Bal_prom = make_shared<promise<vector<uint64_t>>>();
      auto lb1Bal_fut = lb1Bal_prom->get_future();
      auto lb1_getBalanceAndCountLBD = 
         [lb1Bal_prom](ReturnMessage<vector<uint64_t>> balances)->void
      {
         lb1Bal_prom->set_value(move(balances.get()));
      };
      lb1.getBalancesAndCount(5, lb1_getBalanceAndCountLBD);

      //
      auto lb2Bal_prom = make_shared<promise<vector<uint64_t>>>();
      auto lb2Bal_fut = lb2Bal_prom->get_future();
      auto lb2_getBalanceAndCountLBD = 
         [lb2Bal_prom](ReturnMessage<vector<uint64_t>> balances)->void
      {
         lb2Bal_prom->set_value(move(balances.get()));
      };
      lb2.getBalancesAndCount(5, lb2_getBalanceAndCountLBD);

      //get tx
      auto tx_prom = make_shared<promise<Tx>>();
      auto tx_fut = tx_prom->get_future();
      auto tx_get = [tx_prom](ReturnMessage<Tx> tx)->void
      {
         tx_prom->set_value(move(tx.get()));
      };
      bdvObj->getTxByHash(firstHash, tx_get);

      //get utxos
      auto utxo_prom = make_shared<promise<vector<UTXO>>>();
      auto utxo_fut = utxo_prom->get_future();
      auto utxo_get = [utxo_prom](ReturnMessage<vector<UTXO>> utxoV)->void
      {
         utxo_prom->set_value(move(utxoV.get()));
      };
      wallet1.getSpendableTxOutListForValue(UINT64_MAX, utxo_get);

      //wait on futures
      auto w1AddrBalances = move(w1AddrBal_fut.get());
      auto w1Balances = move(w1Bal_fut.get());
      auto lb1AddrBalances = move(lb1AddrBal_fut.get());
      auto lb2AddrBalances = move(lb2AddrBal_fut.get());
      auto lb1Balances = move(lb1Bal_fut.get());
      auto lb2Balances = move(lb2Bal_fut.get());
      auto ledgers = move(ledger_fut.get());
      auto tx = move(tx_fut.get());
      auto utxos = move(utxo_fut.get());

      //w1 addr balances
      auto balanceVec = w1AddrBalances[TestChain::scrAddrA];
      EXPECT_EQ(balanceVec[0], 50 * COIN);
      balanceVec = w1AddrBalances[TestChain::scrAddrB];
      EXPECT_EQ(balanceVec[0], 70 * COIN);
      balanceVec = w1AddrBalances[TestChain::scrAddrC];
      EXPECT_EQ(balanceVec[0], 20 * COIN);

      //w1 balances
      auto fullBalance = w1Balances[0];
      auto spendableBalance = w1Balances[1];
      auto unconfirmedBalance = w1Balances[2];
      EXPECT_EQ(fullBalance, 170 * COIN);
      EXPECT_EQ(spendableBalance, 70 * COIN);
      EXPECT_EQ(unconfirmedBalance, 170 * COIN);

      //lb1 addr balances
      balanceVec = lb1AddrBalances[TestChain::lb1ScrAddr];
      EXPECT_EQ(balanceVec[0], 5 * COIN);
      balanceVec = lb1AddrBalances[TestChain::lb1ScrAddrP2SH];
      EXPECT_EQ(balanceVec[0], 25 * COIN);

      //lb2 addr balances
      balanceVec = lb2AddrBalances[TestChain::lb2ScrAddr];
      EXPECT_EQ(balanceVec[0], 30 * COIN);
      balanceVec = lb2AddrBalances[TestChain::lb2ScrAddrP2SH];
      EXPECT_EQ(balanceVec.size(), 0);

      //lb1 balances
      EXPECT_EQ(lb1Balances[0], 30 * COIN);

      //lb2 balances
      EXPECT_EQ(lb2Balances[0], 30 * COIN);

      //grab main ledgers
      auto& firstEntry = ledgers[1];
      auto txHash = firstEntry.getTxHash();
      EXPECT_EQ(firstHash, txHash);

      //check first tx
      EXPECT_EQ(tx.getThisHash(), firstHash);

      //check utxos
      EXPECT_EQ(utxos.size(), 5);

      //grab all tx for each utxo
      map<BinaryData, shared_future<Tx>> futMap;
      for(auto& utxo : utxos)
      {
         auto& hash = utxo.getTxHash();
         if (futMap.find(hash) != futMap.end())
            continue;

         auto utxoProm = make_shared<promise<Tx>>();
         futMap.insert(make_pair(hash, utxoProm->get_future()));
         auto utxoLBD = [utxoProm](ReturnMessage<Tx> tx)->void
         {
            utxoProm->set_value(move(tx.get()));
         };
         bdvObj->getTxByHash(hash, utxoLBD);
      }

      //third termination spot
      if (checkForTermination())
      {
         cout << "out at third spot" << endl;
         return;
      }

      for(auto& fut_pair : futMap)
      {
         auto txobj = move(fut_pair.second.get());
         EXPECT_EQ(txobj.getThisHash(), fut_pair.first);
      }

      for (auto& thr : delThr)
      {
         if (thr.joinable())
            thr.join();
      }

      for (unsigned i = 0; i < 6; i++)
         EXPECT_EQ(addrLedgerV[i]->size(), 0);
      EXPECT_EQ(addrLedgerV[6]->size(), 1);
      EXPECT_EQ(addrLedgerV[7]->size(), 7);
      EXPECT_EQ(addrLedgerV[8]->size(), 4);
      EXPECT_EQ(addrLedgerV[9]->size(), 2);
      EXPECT_EQ(addrLedgerV[20]->size(), 4);

      for (unsigned i = 0; i < 10; i++)
      {
         auto& v1 = addrLedgerV[i];         
         auto& v2 = addrLedgerV[i + 10];

         if (v1->size() != v2->size())
            EXPECT_TRUE(false);

         for (unsigned y = 0; y < v1->size(); y++)
         {
            if(!((*v1)[y] == (*v2)[y]))
               EXPECT_TRUE(false);
         }
      }

      auto rekeyCount = bdvObj->getRekeyCount();
      EXPECT_EQ(rekeyCount.first, 2);
      EXPECT_TRUE(rekeyCount.second > 7);
      */
      bdvObj->unregisterFromDB();
   };

   vector<thread> thrV;
   for(unsigned ct=0; ct<nThreads; ct++)
      thrV.push_back(thread(request_lambda));

   for(auto& thr : thrV)
   {
      if(thr.joinable())
         thr.join();
   }

   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_ZcUpdate)
{
   //public server
   startupBIP150CTX(4, true);

   /*
   Some sigs in static test chain are borked. P2SH scripts are borked too. This
   test plucks transactions from the static chain to push as ZC. Skip sig checks
   on the unit test mock P2P node to avoid faililng the test.
   */
   nodePtr_->checkSigs(false);

   TestUtils::setBlocks({ "0", "1" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);

   theBDMt_->start(config.initMode_);

   auto pCallback = make_shared<DBTestUtils::UTCallback>();
   auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
   bdvObj->addPublicKey(serverPubkey);
   bdvObj->connectToRemote();
   bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

   //go online
   bdvObj->goOnline();
   pCallback->waitOnSignal(BDMAction_Ready);

   vector<string> walletRegIDs;

   auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
   walletRegIDs.push_back(
      wallet1.registerAddresses(scrAddrVec, false));

   //wait on registration ack
   pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

   //get wallets delegate
   auto del1_prom = make_shared<promise<AsyncClient::LedgerDelegate>>();
   auto del1_fut = del1_prom->get_future();
   auto del1_get = [del1_prom](
      ReturnMessage<AsyncClient::LedgerDelegate> delegate)->void
   {
      del1_prom->set_value(move(delegate.get()));
   };
   bdvObj->getLedgerDelegateForWallets(del1_get);
   auto&& main_delegate = del1_fut.get();

   auto ledger_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger_fut = ledger_prom->get_future();
   auto ledger_get =
      [ledger_prom](
         ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger_get);
   auto&& main_ledger = ledger_fut.get();

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 2);

   EXPECT_EQ(main_ledger[0].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[0].getIndex(), 0);

   EXPECT_EQ(main_ledger[1].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[1].getIndex(), 0);

   //add the 2 zc
   auto&& ZC1 = TestUtils::getTx(2, 1); //block 2, tx 1
   auto&& ZChash1 = BtcUtils::getHash256(ZC1);

   auto&& ZC2 = TestUtils::getTx(2, 2); //block 2, tx 2
   auto&& ZChash2 = BtcUtils::getHash256(ZC2);

   vector<BinaryData> zcVec = {ZC1, ZC2};
   auto broadcastID = bdvObj->broadcastZC(zcVec);
   
   {
      set<BinaryData> zcHashes = { ZChash1, ZChash2 };
      set<BinaryData> scrAddrSet;

      Tx zctx1(ZC1);
      for (unsigned i = 0; i < zctx1.getNumTxOut(); i++)
         scrAddrSet.insert(zctx1.getScrAddrForTxOut(i));

      Tx zctx2(ZC2);
      for (unsigned i = 0; i < zctx2.getNumTxOut(); i++)
         scrAddrSet.insert(zctx2.getScrAddrForTxOut(i));

      pCallback->waitOnZc(zcHashes, scrAddrSet, broadcastID);
   }

   //get the new ledgers
   auto ledger2_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger2_fut = ledger2_prom->get_future();
   auto ledger2_get =
      [ledger2_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger2_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger2_get);
   main_ledger = move(ledger2_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 4);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[0].getIndex(), 1);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[1].getIndex(), 0);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   //tx cache testing
   //grab ZC1 from async client
   auto zc_prom1 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut1 = zc_prom1->get_future();
   auto zc_get1 =
      [zc_prom1](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom1->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get1);
   auto zc_obj1 = zc_fut1.get();
   EXPECT_EQ(ZChash1, zc_obj1->getThisHash());
   EXPECT_EQ(zc_obj1->getTxHeight(), UINT32_MAX);

   //grab both zc from async client
   auto zc_prom2 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut2 = zc_prom2->get_future();
   auto zc_get2 =
      [zc_prom2](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom2->set_value(move(txVec));
   };

   set<BinaryData> bothZC = { ZChash1, ZChash2 };
   bdvObj->getTxBatchByHash(bothZC, zc_get2);
   auto zc_obj2 = zc_fut2.get();

   ASSERT_EQ(zc_obj2.size(), 2);

   auto iterZc1 = zc_obj2.find(ZChash1);
   ASSERT_NE(iterZc1, zc_obj2.end());
   ASSERT_NE(iterZc1->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc1->second->getThisHash());
   EXPECT_EQ(iterZc1->second->getTxHeight(), UINT32_MAX);

   auto iterZc2 = zc_obj2.find(ZChash2);
   ASSERT_NE(iterZc2, zc_obj2.end());
   ASSERT_NE(iterZc2->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc2->second->getThisHash());
   EXPECT_EQ(iterZc2->second->getTxHeight(), UINT32_MAX);

   //push an extra block
   TestUtils::appendBlocks({ "2" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   pCallback->waitOnSignal(BDMAction_NewBlock);

   //get the new ledgers
   auto ledger3_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger3_fut = ledger3_prom->get_future();
   auto ledger3_get =
      [ledger3_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger3_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger3_get);
   main_ledger = move(ledger3_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 5);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[0].getIndex(), 2);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[1].getIndex(), 1);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   EXPECT_EQ(main_ledger[4].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[4].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[4].getIndex(), 0);


   //grab ZC1 from async client
   auto zc_prom3 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut3 = zc_prom3->get_future();
   auto zc_get3 =
      [zc_prom3](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom3->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get3);
   auto zc_obj3 = zc_fut3.get();
   EXPECT_EQ(ZChash1, zc_obj3->getThisHash());
   EXPECT_EQ(zc_obj3->getTxHeight(), 2);

   //grab both zc from async client
   auto zc_prom4 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut4 = zc_prom4->get_future();
   auto zc_get4 =
      [zc_prom4](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom4->set_value(move(txVec));
   };

   bdvObj->getTxBatchByHash(bothZC, zc_get4);
   auto zc_obj4 = zc_fut4.get();

   ASSERT_EQ(zc_obj4.size(), 2);

   auto iterZc3 = zc_obj4.find(ZChash1);
   ASSERT_NE(iterZc3, zc_obj4.end());
   ASSERT_NE(iterZc3->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc3->second->getThisHash());
   EXPECT_EQ(iterZc3->second->getTxHeight(), 2);

   auto iterZc4 = zc_obj4.find(ZChash2);
   ASSERT_NE(iterZc4, zc_obj4.end());
   ASSERT_NE(iterZc4->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc4->second->getThisHash());
   EXPECT_EQ(iterZc4->second->getTxHeight(), 2);

   //disconnect
   bdvObj->unregisterFromDB();

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_ZcUpdate_RPC)
{
   //public server
   startupBIP150CTX(4, true);

   /*
   Some sigs in static test chain are borked. P2SH scripts are borked too. This
   test plucks transactions from the static chain to push as ZC. Skip sig checks
   on the unit test mock P2P node to avoid faililng the test.
   */
   nodePtr_->checkSigs(false);

   TestUtils::setBlocks({ "0", "1" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);

   theBDMt_->start(config.initMode_);

   auto pCallback = make_shared<DBTestUtils::UTCallback>();
   auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
   bdvObj->addPublicKey(serverPubkey);
   bdvObj->connectToRemote();
   bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

   //go online
   bdvObj->goOnline();
   pCallback->waitOnSignal(BDMAction_Ready);

   vector<string> walletRegIDs;

   auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
   walletRegIDs.push_back(
      wallet1.registerAddresses(scrAddrVec, false));

   //wait on registration ack
   pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

   //get wallets delegate
   auto del1_prom = make_shared<promise<AsyncClient::LedgerDelegate>>();
   auto del1_fut = del1_prom->get_future();
   auto del1_get = [del1_prom](
      ReturnMessage<AsyncClient::LedgerDelegate> delegate)->void
   {
      del1_prom->set_value(move(delegate.get()));
   };
   bdvObj->getLedgerDelegateForWallets(del1_get);
   auto&& main_delegate = del1_fut.get();

   auto ledger_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger_fut = ledger_prom->get_future();
   auto ledger_get =
      [ledger_prom](
         ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger_get);
   auto&& main_ledger = ledger_fut.get();

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 2);

   EXPECT_EQ(main_ledger[0].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[0].getIndex(), 0);

   EXPECT_EQ(main_ledger[1].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[1].getIndex(), 0);

   //add the 2 zc
   auto&& ZC1 = TestUtils::getTx(2, 1); //block 2, tx 1
   auto&& ZChash1 = BtcUtils::getHash256(ZC1);

   auto&& ZC2 = TestUtils::getTx(2, 2); //block 2, tx 2
   auto&& ZChash2 = BtcUtils::getHash256(ZC2);

   auto broadcastId1 = bdvObj->broadcastThroughRPC(ZC1);
   auto broadcastId2 = bdvObj->broadcastThroughRPC(ZC2);
   
   {
      set<BinaryData> zcHashes = { ZChash1, ZChash2 };
      set<BinaryData> scrAddrSet1, scrAddrSet2;

      Tx zctx1(ZC1);
      for (unsigned i = 0; i < zctx1.getNumTxOut(); i++)
         scrAddrSet1.insert(zctx1.getScrAddrForTxOut(i));

      Tx zctx2(ZC2);
      for (unsigned i = 0; i < zctx2.getNumTxOut(); i++)
         scrAddrSet2.insert(zctx2.getScrAddrForTxOut(i));

      pCallback->waitOnZc({ZChash1}, scrAddrSet1, broadcastId1);
      pCallback->waitOnZc({ZChash2}, scrAddrSet2, broadcastId2);
   }

   //get the new ledgers
   auto ledger2_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger2_fut = ledger2_prom->get_future();
   auto ledger2_get =
      [ledger2_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger2_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger2_get);
   main_ledger = move(ledger2_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 4);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[0].getIndex(), 1);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[1].getIndex(), 0);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   /*tx cache coverage*/
   //grab ZC1 from async client
   auto zc_prom1 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut1 = zc_prom1->get_future();
   auto zc_get1 =
      [zc_prom1](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom1->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get1);
   auto zc_obj1 = zc_fut1.get();
   EXPECT_EQ(ZChash1, zc_obj1->getThisHash());
   EXPECT_EQ(zc_obj1->getTxHeight(), UINT32_MAX);

   //grab both zc from async client
   auto zc_prom2 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut2 = zc_prom2->get_future();
   auto zc_get2 =
      [zc_prom2](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom2->set_value(move(txVec));
   };

   set<BinaryData> bothZC = { ZChash1, ZChash2 };
   bdvObj->getTxBatchByHash(bothZC, zc_get2);
   auto zc_obj2 = zc_fut2.get();

   ASSERT_EQ(zc_obj2.size(), 2);
   
   auto iterZc1 = zc_obj2.find(ZChash1);
   ASSERT_NE(iterZc1, zc_obj2.end());
   ASSERT_NE(iterZc1->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc1->second->getThisHash());
   EXPECT_EQ(iterZc1->second->getTxHeight(), UINT32_MAX);

   auto iterZc2 = zc_obj2.find(ZChash2);
   ASSERT_NE(iterZc2, zc_obj2.end());
   ASSERT_NE(iterZc2->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc2->second->getThisHash());
   EXPECT_EQ(iterZc2->second->getTxHeight(), UINT32_MAX);

   //push an extra block
   TestUtils::appendBlocks({ "2" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   pCallback->waitOnSignal(BDMAction_NewBlock);

   //get the new ledgers
   auto ledger3_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger3_fut = ledger3_prom->get_future();
   auto ledger3_get =
      [ledger3_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger3_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger3_get);
   main_ledger = move(ledger3_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 5);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[0].getIndex(), 2);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[1].getIndex(), 1);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   EXPECT_EQ(main_ledger[4].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[4].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[4].getIndex(), 0);


   //grab ZC1 from async client
   auto zc_prom3 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut3 = zc_prom3->get_future();
   auto zc_get3 =
      [zc_prom3](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom3->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get3);
   auto zc_obj3 = zc_fut3.get();
   EXPECT_EQ(ZChash1, zc_obj3->getThisHash());
   EXPECT_EQ(zc_obj3->getTxHeight(), 2);

   //grab both zc from async client
   auto zc_prom4 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut4 = zc_prom4->get_future();
   auto zc_get4 =
      [zc_prom4](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom4->set_value(move(txVec));
   };

   bdvObj->getTxBatchByHash(bothZC, zc_get4);
   auto zc_obj4 = zc_fut4.get();

   ASSERT_EQ(zc_obj4.size(), 2);
   auto iterZc3 = zc_obj4.find(ZChash1);
   ASSERT_NE(iterZc3, zc_obj4.end());
   ASSERT_NE(iterZc3->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc3->second->getThisHash());
   EXPECT_EQ(iterZc3->second->getTxHeight(), 2);

   auto iterZc4 = zc_obj4.find(ZChash2);
   ASSERT_NE(iterZc4, zc_obj4.end());
   ASSERT_NE(iterZc4->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc4->second->getThisHash());
   EXPECT_EQ(iterZc4->second->getTxHeight(), 2);

   //disconnect
   bdvObj->unregisterFromDB();

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_ZcUpdate_RPC_Fallback)
{
   //public server
   startupBIP150CTX(4, true);

   /*
   Some sigs in static test chain are borked. P2SH scripts are borked too. This
   test plucks transactions from the static chain to push as ZC. Skip sig checks
   on the unit test mock P2P node to avoid faililng the test.
   */
   nodePtr_->checkSigs(false);

   TestUtils::setBlocks({ "0", "1" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);

   theBDMt_->start(config.initMode_);

   auto pCallback = make_shared<DBTestUtils::UTCallback>();
   auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
   bdvObj->addPublicKey(serverPubkey);
   bdvObj->connectToRemote();
   bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

   //go online
   bdvObj->goOnline();
   pCallback->waitOnSignal(BDMAction_Ready);

   vector<string> walletRegIDs;

   auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
   walletRegIDs.push_back(
      wallet1.registerAddresses(scrAddrVec, false));

   //wait on registration ack
   pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

   //get wallets delegate
   auto del1_prom = make_shared<promise<AsyncClient::LedgerDelegate>>();
   auto del1_fut = del1_prom->get_future();
   auto del1_get = [del1_prom](
      ReturnMessage<AsyncClient::LedgerDelegate> delegate)->void
   {
      del1_prom->set_value(move(delegate.get()));
   };
   bdvObj->getLedgerDelegateForWallets(del1_get);
   auto&& main_delegate = del1_fut.get();

   auto ledger_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger_fut = ledger_prom->get_future();
   auto ledger_get =
      [ledger_prom](
         ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger_get);
   auto&& main_ledger = ledger_fut.get();

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 2);

   EXPECT_EQ(main_ledger[0].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[0].getIndex(), 0);

   EXPECT_EQ(main_ledger[1].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[1].getIndex(), 0);

   //add the 2 zc
   auto&& ZC1 = TestUtils::getTx(2, 1); //block 2, tx 1
   auto&& ZChash1 = BtcUtils::getHash256(ZC1);

   auto&& ZC2 = TestUtils::getTx(2, 2); //block 2, tx 2
   auto&& ZChash2 = BtcUtils::getHash256(ZC2);

   //both these zc will be skipped by the p2p broadcast interface,
   //should trigger a RPC broadcast
   nodePtr_->skipZc(2);
   auto broadcastId1 = bdvObj->broadcastZC(ZC1);
   auto broadcastId2 = bdvObj->broadcastZC(ZC2);
   
   {
      set<BinaryData> scrAddrSet1, scrAddrSet2;

      Tx zctx1(ZC1);
      for (unsigned i = 0; i < zctx1.getNumTxOut(); i++)
         scrAddrSet1.insert(zctx1.getScrAddrForTxOut(i));

      Tx zctx2(ZC2);
      for (unsigned i = 0; i < zctx2.getNumTxOut(); i++)
         scrAddrSet2.insert(zctx2.getScrAddrForTxOut(i));

      pCallback->waitOnZc({ZChash1}, scrAddrSet1, broadcastId1);
      pCallback->waitOnZc({ZChash2}, scrAddrSet2, broadcastId2);
   }

   //get the new ledgers
   auto ledger2_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger2_fut = ledger2_prom->get_future();
   auto ledger2_get =
      [ledger2_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger2_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger2_get);
   main_ledger = move(ledger2_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 4);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[0].getIndex(), 3);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[1].getIndex(), 2);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   //tx cache testing
   //grab ZC1 from async client
   auto zc_prom1 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut1 = zc_prom1->get_future();
   auto zc_get1 =
      [zc_prom1](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom1->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get1);
   auto zc_obj1 = zc_fut1.get();
   EXPECT_EQ(ZChash1, zc_obj1->getThisHash());
   EXPECT_EQ(zc_obj1->getTxHeight(), UINT32_MAX);

   //grab both zc from async client
   auto zc_prom2 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut2 = zc_prom2->get_future();
   auto zc_get2 =
      [zc_prom2](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom2->set_value(move(txVec));
   };

   set<BinaryData> bothZC = { ZChash1, ZChash2 };
   bdvObj->getTxBatchByHash(bothZC, zc_get2);
   auto zc_obj2 = zc_fut2.get();

   ASSERT_EQ(zc_obj2.size(), 2);

   auto iterZc1 = zc_obj2.find(ZChash1);
   ASSERT_NE(iterZc1, zc_obj2.end());
   ASSERT_NE(iterZc1->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc1->second->getThisHash());
   EXPECT_EQ(iterZc1->second->getTxHeight(), UINT32_MAX);

   auto iterZc2 = zc_obj2.find(ZChash2);
   ASSERT_NE(iterZc2, zc_obj2.end());
   ASSERT_NE(iterZc2->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc2->second->getThisHash());
   EXPECT_EQ(iterZc2->second->getTxHeight(), UINT32_MAX);

   //push an extra block
   TestUtils::appendBlocks({ "2" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   pCallback->waitOnSignal(BDMAction_NewBlock);

   //get the new ledgers
   auto ledger3_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger3_fut = ledger3_prom->get_future();
   auto ledger3_get =
      [ledger3_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger3_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger3_get);
   main_ledger = move(ledger3_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 5);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[0].getIndex(), 2);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[1].getIndex(), 1);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   EXPECT_EQ(main_ledger[4].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[4].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[4].getIndex(), 0);


   //grab ZC1 from async client
   auto zc_prom3 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut3 = zc_prom3->get_future();
   auto zc_get3 =
      [zc_prom3](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom3->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get3);
   auto zc_obj3 = zc_fut3.get();
   EXPECT_EQ(ZChash1, zc_obj3->getThisHash());
   EXPECT_EQ(zc_obj3->getTxHeight(), 2);

   //grab both zc from async client
   auto zc_prom4 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut4 = zc_prom4->get_future();
   auto zc_get4 =
      [zc_prom4](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom4->set_value(move(txVec));
   };

   bdvObj->getTxBatchByHash(bothZC, zc_get4);
   auto zc_obj4 = zc_fut4.get();

   ASSERT_EQ(zc_obj4.size(), 2);

   auto iterZc3 = zc_obj4.find(ZChash1);
   ASSERT_NE(iterZc3, zc_obj4.end());
   ASSERT_NE(iterZc3->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc3->second->getThisHash());
   EXPECT_EQ(iterZc3->second->getTxHeight(), 2);

   auto iterZc4 = zc_obj4.find(ZChash2);
   ASSERT_NE(iterZc4, zc_obj4.end());
   ASSERT_NE(iterZc4->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc4->second->getThisHash());
   EXPECT_EQ(iterZc4->second->getTxHeight(), 2);

   //disconnect
   bdvObj->unregisterFromDB();

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_ZcUpdate_RPC_Fallback_SingleBatch)
{
   //public server
   startupBIP150CTX(4, true);

   /*
   Some sigs in static test chain are borked. P2SH scripts are borked too. This
   test plucks transactions from the static chain to push as ZC. Skip sig checks
   on the unit test mock P2P node to avoid faililng the test.
   */
   nodePtr_->checkSigs(false);

   TestUtils::setBlocks({ "0", "1" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);

   theBDMt_->start(config.initMode_);

   auto pCallback = make_shared<DBTestUtils::UTCallback>();
   auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
   bdvObj->addPublicKey(serverPubkey);
   bdvObj->connectToRemote();
   bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

   //go online
   bdvObj->goOnline();
   pCallback->waitOnSignal(BDMAction_Ready);

   vector<string> walletRegIDs;

   auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
   walletRegIDs.push_back(
      wallet1.registerAddresses(scrAddrVec, false));

   //wait on registration ack
   pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

   //get wallets delegate
   auto del1_prom = make_shared<promise<AsyncClient::LedgerDelegate>>();
   auto del1_fut = del1_prom->get_future();
   auto del1_get = [del1_prom](
      ReturnMessage<AsyncClient::LedgerDelegate> delegate)->void
   {
      del1_prom->set_value(move(delegate.get()));
   };
   bdvObj->getLedgerDelegateForWallets(del1_get);
   auto&& main_delegate = del1_fut.get();

   auto ledger_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger_fut = ledger_prom->get_future();
   auto ledger_get =
      [ledger_prom](
         ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger_get);
   auto&& main_ledger = ledger_fut.get();

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 2);

   EXPECT_EQ(main_ledger[0].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[0].getIndex(), 0);

   EXPECT_EQ(main_ledger[1].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[1].getIndex(), 0);

   //add the 2 zc
   auto&& ZC1 = TestUtils::getTx(2, 1); //block 2, tx 1
   auto&& ZChash1 = BtcUtils::getHash256(ZC1);

   auto&& ZC2 = TestUtils::getTx(2, 2); //block 2, tx 2
   auto&& ZChash2 = BtcUtils::getHash256(ZC2);

   //both these zc will be skipped by the p2p broadcast interface,
   //should trigger a RPC broadcast
   nodePtr_->skipZc(2);
   vector<BinaryData> zcVec = {ZC1, ZC2};
   auto broadcastId1 = bdvObj->broadcastZC(zcVec);
   
   {
      set<BinaryData> zcHashes = { ZChash1, ZChash2 };
      set<BinaryData> scrAddrSet;

      Tx zctx1(ZC1);
      for (unsigned i = 0; i < zctx1.getNumTxOut(); i++)
         scrAddrSet.insert(zctx1.getScrAddrForTxOut(i));

      Tx zctx2(ZC2);
      for (unsigned i = 0; i < zctx2.getNumTxOut(); i++)
         scrAddrSet.insert(zctx2.getScrAddrForTxOut(i));

      pCallback->waitOnZc(zcHashes, scrAddrSet, broadcastId1);
   }

   //get the new ledgers
   auto ledger2_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger2_fut = ledger2_prom->get_future();
   auto ledger2_get =
      [ledger2_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger2_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger2_get);
   main_ledger = move(ledger2_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 4);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[0].getIndex(), 3);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[1].getIndex(), 2);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   //tx cache testing
   //grab ZC1 from async client
   auto zc_prom1 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut1 = zc_prom1->get_future();
   auto zc_get1 =
      [zc_prom1](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom1->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get1);
   auto zc_obj1 = zc_fut1.get();
   EXPECT_EQ(ZChash1, zc_obj1->getThisHash());
   EXPECT_EQ(zc_obj1->getTxHeight(), UINT32_MAX);

   //grab both zc from async client
   auto zc_prom2 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut2 = zc_prom2->get_future();
   auto zc_get2 =
      [zc_prom2](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom2->set_value(move(txVec));
   };

   set<BinaryData> bothZC = { ZChash1, ZChash2 };
   bdvObj->getTxBatchByHash(bothZC, zc_get2);
   auto zc_obj2 = zc_fut2.get();

   ASSERT_EQ(zc_obj2.size(), 2);

   auto iterZc1 = zc_obj2.find(ZChash1);
   ASSERT_NE(iterZc1, zc_obj2.end());
   ASSERT_NE(iterZc1->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc1->second->getThisHash());
   EXPECT_EQ(iterZc1->second->getTxHeight(), UINT32_MAX);

   auto iterZc2 = zc_obj2.find(ZChash2);
   ASSERT_NE(iterZc2, zc_obj2.end());
   ASSERT_NE(iterZc2->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc2->second->getThisHash());
   EXPECT_EQ(iterZc2->second->getTxHeight(), UINT32_MAX);

   //push an extra block
   TestUtils::appendBlocks({ "2" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   pCallback->waitOnSignal(BDMAction_NewBlock);

   //get the new ledgers
   auto ledger3_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger3_fut = ledger3_prom->get_future();
   auto ledger3_get =
      [ledger3_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger3_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger3_get);
   main_ledger = move(ledger3_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 5);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[0].getIndex(), 2);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[1].getIndex(), 1);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   EXPECT_EQ(main_ledger[4].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[4].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[4].getIndex(), 0);


   //grab ZC1 from async client
   auto zc_prom3 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut3 = zc_prom3->get_future();
   auto zc_get3 =
      [zc_prom3](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom3->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get3);
   auto zc_obj3 = zc_fut3.get();
   EXPECT_EQ(ZChash1, zc_obj3->getThisHash());
   EXPECT_EQ(zc_obj3->getTxHeight(), 2);

   //grab both zc from async client
   auto zc_prom4 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut4 = zc_prom4->get_future();
   auto zc_get4 =
      [zc_prom4](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom4->set_value(move(txVec));
   };

   bdvObj->getTxBatchByHash(bothZC, zc_get4);
   auto zc_obj4 = zc_fut4.get();

   ASSERT_EQ(zc_obj4.size(), 2);

   auto iterZc3 = zc_obj4.find(ZChash1);
   ASSERT_NE(iterZc3, zc_obj4.end());
   ASSERT_NE(iterZc3->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc3->second->getThisHash());
   EXPECT_EQ(iterZc3->second->getTxHeight(), 2);

   auto iterZc4 = zc_obj4.find(ZChash2);
   ASSERT_NE(iterZc4, zc_obj4.end());
   ASSERT_NE(iterZc4->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc4->second->getThisHash());
   EXPECT_EQ(iterZc4->second->getTxHeight(), 2);

   //disconnect
   bdvObj->unregisterFromDB();

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_ZcUpdate_AlreadyInMempool)
{
   //public server
   startupBIP150CTX(4, true);

   /*
   Some sigs in static test chain are borked. P2SH scripts are borked too. This
   test plucks transactions from the static chain to push as ZC. Skip sig checks
   on the unit test mock P2P node to avoid faililng the test.
   */
   nodePtr_->checkSigs(false);

   TestUtils::setBlocks({ "0", "1" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);

   theBDMt_->start(config.initMode_);

   auto pCallback = make_shared<DBTestUtils::UTCallback>();
   auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
   bdvObj->addPublicKey(serverPubkey);
   bdvObj->connectToRemote();
   bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

   //go online
   bdvObj->goOnline();
   pCallback->waitOnSignal(BDMAction_Ready);

   vector<string> walletRegIDs;

   auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
   walletRegIDs.push_back(
      wallet1.registerAddresses(scrAddrVec, false));

   //wait on registration ack
   pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

   //get wallets delegate
   auto del1_prom = make_shared<promise<AsyncClient::LedgerDelegate>>();
   auto del1_fut = del1_prom->get_future();
   auto del1_get = [del1_prom](
      ReturnMessage<AsyncClient::LedgerDelegate> delegate)->void
   {
      del1_prom->set_value(move(delegate.get()));
   };
   bdvObj->getLedgerDelegateForWallets(del1_get);
   auto&& main_delegate = del1_fut.get();

   auto ledger_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger_fut = ledger_prom->get_future();
   auto ledger_get =
      [ledger_prom](
         ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger_get);
   auto&& main_ledger = ledger_fut.get();

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 2);

   EXPECT_EQ(main_ledger[0].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[0].getIndex(), 0);

   EXPECT_EQ(main_ledger[1].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[1].getIndex(), 0);

   //add the 2 zc
   auto&& ZC1 = TestUtils::getTx(2, 1); //block 2, tx 1
   auto&& ZChash1 = BtcUtils::getHash256(ZC1);

   auto&& ZC2 = TestUtils::getTx(2, 2); //block 2, tx 2
   auto&& ZChash2 = BtcUtils::getHash256(ZC2);

   //pushZC
   auto broadcastId1 = bdvObj->broadcastZC(ZC1);
   auto broadcastId2 = bdvObj->broadcastZC(ZC2);
   
   {
      set<BinaryData> scrAddrSet1, scrAddrSet2;

      Tx zctx1(ZC1);
      for (unsigned i = 0; i < zctx1.getNumTxOut(); i++)
         scrAddrSet1.insert(zctx1.getScrAddrForTxOut(i));

      Tx zctx2(ZC2);
      for (unsigned i = 0; i < zctx2.getNumTxOut(); i++)
         scrAddrSet2.insert(zctx2.getScrAddrForTxOut(i));

      pCallback->waitOnZc({ZChash1}, scrAddrSet1, broadcastId1);
      pCallback->waitOnZc({ZChash2}, scrAddrSet2, broadcastId2);
   }

   //push them again, should get already in mempool error
   auto broadcastId3 = bdvObj->broadcastZC(ZC1);
   auto broadcastId4 = bdvObj->broadcastZC(ZC2);

   pCallback->waitOnError(
      ZChash1, ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool, broadcastId3);
   pCallback->waitOnError(
      ZChash2, ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool, broadcastId4);

   //get the new ledgers
   auto ledger2_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger2_fut = ledger2_prom->get_future();
   auto ledger2_get =
      [ledger2_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger2_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger2_get);
   main_ledger = move(ledger2_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 4);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[0].getIndex(), 1);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[1].getIndex(), 0);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   //tx cache testing
   //grab ZC1 from async client
   auto zc_prom1 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut1 = zc_prom1->get_future();
   auto zc_get1 =
      [zc_prom1](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom1->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get1);
   auto zc_obj1 = zc_fut1.get();
   EXPECT_EQ(ZChash1, zc_obj1->getThisHash());
   EXPECT_EQ(zc_obj1->getTxHeight(), UINT32_MAX);

   //grab both zc from async client
   auto zc_prom2 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut2 = zc_prom2->get_future();
   auto zc_get2 =
      [zc_prom2](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom2->set_value(move(txVec));
   };

   set<BinaryData> bothZC = { ZChash1, ZChash2 };
   bdvObj->getTxBatchByHash(bothZC, zc_get2);
   auto zc_obj2 = zc_fut2.get();

   ASSERT_EQ(zc_obj2.size(), 2);

   auto iterZc1 = zc_obj2.find(ZChash1);
   ASSERT_NE(iterZc1, zc_obj2.end());
   ASSERT_NE(iterZc1->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc1->second->getThisHash());
   EXPECT_EQ(iterZc1->second->getTxHeight(), UINT32_MAX);

   auto iterZc2 = zc_obj2.find(ZChash2);
   ASSERT_NE(iterZc2, zc_obj2.end());
   ASSERT_NE(iterZc2->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc2->second->getThisHash());
   EXPECT_EQ(iterZc2->second->getTxHeight(), UINT32_MAX);

   //push an extra block
   TestUtils::appendBlocks({ "2" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   pCallback->waitOnSignal(BDMAction_NewBlock);

   //get the new ledgers
   auto ledger3_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger3_fut = ledger3_prom->get_future();
   auto ledger3_get =
      [ledger3_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger3_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger3_get);
   main_ledger = move(ledger3_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 5);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[0].getIndex(), 2);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[1].getIndex(), 1);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   EXPECT_EQ(main_ledger[4].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[4].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[4].getIndex(), 0);


   //grab ZC1 from async client
   auto zc_prom3 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut3 = zc_prom3->get_future();
   auto zc_get3 =
      [zc_prom3](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom3->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get3);
   auto zc_obj3 = zc_fut3.get();
   EXPECT_EQ(ZChash1, zc_obj3->getThisHash());
   EXPECT_EQ(zc_obj3->getTxHeight(), 2);

   //grab both zc from async client
   auto zc_prom4 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut4 = zc_prom4->get_future();
   auto zc_get4 =
      [zc_prom4](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom4->set_value(move(txVec));
   };

   bdvObj->getTxBatchByHash(bothZC, zc_get4);
   auto zc_obj4 = zc_fut4.get();

   ASSERT_EQ(zc_obj4.size(), 2);

   auto iterZc3 = zc_obj4.find(ZChash1);
   ASSERT_NE(iterZc3, zc_obj4.end());
   ASSERT_NE(iterZc3->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc3->second->getThisHash());
   EXPECT_EQ(iterZc3->second->getTxHeight(), 2);

   auto iterZc4 = zc_obj4.find(ZChash2);
   ASSERT_NE(iterZc4, zc_obj4.end());
   ASSERT_NE(iterZc4->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc4->second->getThisHash());
   EXPECT_EQ(iterZc4->second->getTxHeight(), 2);

   //disconnect
   bdvObj->unregisterFromDB();

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_ZcUpdate_AlreadyInMempool_Batched)
{
   //public server
   startupBIP150CTX(4, true);

   /*
   Some sigs in static test chain are borked. P2SH scripts are borked too. This
   test plucks transactions from the static chain to push as ZC. Skip sig checks
   on the unit test mock P2P node to avoid faililng the test.
   */
   nodePtr_->checkSigs(false);

   TestUtils::setBlocks({ "0", "1" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);

   theBDMt_->start(config.initMode_);

   auto pCallback = make_shared<DBTestUtils::UTCallback>();
   auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
   bdvObj->addPublicKey(serverPubkey);
   bdvObj->connectToRemote();
   bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

   //go online
   bdvObj->goOnline();
   pCallback->waitOnSignal(BDMAction_Ready);

   vector<string> walletRegIDs;

   auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
   walletRegIDs.push_back(
      wallet1.registerAddresses(scrAddrVec, false));

   //wait on registration ack
   pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

   //get wallets delegate
   auto del1_prom = make_shared<promise<AsyncClient::LedgerDelegate>>();
   auto del1_fut = del1_prom->get_future();
   auto del1_get = [del1_prom](
      ReturnMessage<AsyncClient::LedgerDelegate> delegate)->void
   {
      del1_prom->set_value(move(delegate.get()));
   };
   bdvObj->getLedgerDelegateForWallets(del1_get);
   auto&& main_delegate = del1_fut.get();

   auto ledger_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger_fut = ledger_prom->get_future();
   auto ledger_get =
      [ledger_prom](
         ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger_get);
   auto&& main_ledger = ledger_fut.get();

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 2);

   EXPECT_EQ(main_ledger[0].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[0].getIndex(), 0);

   EXPECT_EQ(main_ledger[1].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[1].getIndex(), 0);

   //add the 2 zc
   auto&& ZC1 = TestUtils::getTx(2, 1); //block 2, tx 1
   auto&& ZChash1 = BtcUtils::getHash256(ZC1);

   auto&& ZC2 = TestUtils::getTx(2, 2); //block 2, tx 2
   auto&& ZChash2 = BtcUtils::getHash256(ZC2);

   //push the first zc
   auto broadcastId1 = bdvObj->broadcastZC(ZC1);
   
   {
      set<BinaryData> zcHashes = { ZChash1 };
      set<BinaryData> scrAddrSet;

      Tx zctx1(ZC1);
      for (unsigned i = 0; i < zctx1.getNumTxOut(); i++)
         scrAddrSet.insert(zctx1.getScrAddrForTxOut(i));

      pCallback->waitOnZc(zcHashes, scrAddrSet, broadcastId1);
   }

   //push them again, should get already in mempool error for first zc, notif for 2nd
   auto broadcastId2 = bdvObj->broadcastZC( { ZC1, ZC2 } );
   pCallback->waitOnError(
      ZChash1, ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool, broadcastId2);

   {
      set<BinaryData> zcHashes = { ZChash2 };
      set<BinaryData> scrAddrSet;

      Tx zctx2(ZC2);
      for (unsigned i = 0; i < zctx2.getNumTxOut(); i++)
         scrAddrSet.insert(zctx2.getScrAddrForTxOut(i));

      pCallback->waitOnZc(zcHashes, scrAddrSet, broadcastId2);
   }

   //get the new ledgers
   auto ledger2_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger2_fut = ledger2_prom->get_future();
   auto ledger2_get =
      [ledger2_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger2_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger2_get);
   main_ledger = move(ledger2_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 4);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), UINT32_MAX);
   //zc index is 2 since 0 and 1 were assigned to the first zc: 0 at
   //the solo broadcast, 1 at the batched broadcast, which had the first
   //zc fail as already-in-mempool
   EXPECT_EQ(main_ledger[0].getIndex(), 2); 

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[1].getIndex(), 0);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   //tx cache testing
   //grab ZC1 from async client
   auto zc_prom1 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut1 = zc_prom1->get_future();
   auto zc_get1 =
      [zc_prom1](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom1->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get1);
   auto zc_obj1 = zc_fut1.get();
   EXPECT_EQ(ZChash1, zc_obj1->getThisHash());
   EXPECT_EQ(zc_obj1->getTxHeight(), UINT32_MAX);

   //grab both zc from async client
   auto zc_prom2 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut2 = zc_prom2->get_future();
   auto zc_get2 =
      [zc_prom2](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom2->set_value(move(txVec));
   };

   set<BinaryData> bothZC = { ZChash1, ZChash2 };
   bdvObj->getTxBatchByHash(bothZC, zc_get2);
   auto zc_obj2 = zc_fut2.get();

   ASSERT_EQ(zc_obj2.size(), 2);

   auto iterZc1 = zc_obj2.find(ZChash1);
   ASSERT_NE(iterZc1, zc_obj2.end());
   ASSERT_NE(iterZc1->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc1->second->getThisHash());
   EXPECT_EQ(iterZc1->second->getTxHeight(), UINT32_MAX);

   auto iterZc2 = zc_obj2.find(ZChash2);
   ASSERT_NE(iterZc2, zc_obj2.end());
   ASSERT_NE(iterZc2->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc2->second->getThisHash());
   EXPECT_EQ(iterZc2->second->getTxHeight(), UINT32_MAX);
   
   //push an extra block
   TestUtils::appendBlocks({ "2" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   pCallback->waitOnSignal(BDMAction_NewBlock);

   //get the new ledgers
   auto ledger3_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger3_fut = ledger3_prom->get_future();
   auto ledger3_get =
      [ledger3_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger3_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger3_get);
   main_ledger = move(ledger3_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 5);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[0].getIndex(), 2);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[1].getIndex(), 1);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   EXPECT_EQ(main_ledger[4].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[4].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[4].getIndex(), 0);


   //grab ZC1 from async client
   auto zc_prom3 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut3 = zc_prom3->get_future();
   auto zc_get3 =
      [zc_prom3](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom3->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get3);
   auto zc_obj3 = zc_fut3.get();
   EXPECT_EQ(ZChash1, zc_obj3->getThisHash());
   EXPECT_EQ(zc_obj3->getTxHeight(), 2);

   //grab both zc from async client
   auto zc_prom4 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut4 = zc_prom4->get_future();
   auto zc_get4 =
      [zc_prom4](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom4->set_value(move(txVec));
   };

   bdvObj->getTxBatchByHash(bothZC, zc_get4);
   auto zc_obj4 = zc_fut4.get();

   ASSERT_EQ(zc_obj4.size(), 2);

   auto iterZc3 = zc_obj4.find(ZChash1);
   ASSERT_NE(iterZc3, zc_obj4.end());
   ASSERT_NE(iterZc3->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc3->second->getThisHash());
   EXPECT_EQ(iterZc3->second->getTxHeight(), 2);

   auto iterZc4 = zc_obj4.find(ZChash2);
   ASSERT_NE(iterZc4, zc_obj4.end());
   ASSERT_NE(iterZc4->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc4->second->getThisHash());
   EXPECT_EQ(iterZc4->second->getTxHeight(), 2);

   //disconnect
   bdvObj->unregisterFromDB();

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_ZcUpdate_AlreadyInNodeMempool)
{
   /*
   Some sigs in static test chain are borked. P2SH scripts are borked too. This
   test plucks transactions from the static chain to push as ZC. Skip sig checks
   on the unit test mock P2P node to avoid faililng the test.
   */
   nodePtr_->checkSigs(false);

   //grab the first zc
   auto&& ZC1 = TestUtils::getTx(2, 1); //block 2, tx 1
   auto&& ZChash1 = BtcUtils::getHash256(ZC1);

   {
      //feed to node mempool while the zc parser is down
      DBTestUtils::ZcVector zcVec;
      zcVec.push_back(ZC1, 0);
      DBTestUtils::pushNewZc(theBDMt_, zcVec, 0);
   }

   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);

   theBDMt_->start(config.initMode_);

   auto pCallback = make_shared<DBTestUtils::UTCallback>();
   auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
   bdvObj->addPublicKey(serverPubkey);
   bdvObj->connectToRemote();
   bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

   //go online
   bdvObj->goOnline();
   pCallback->waitOnSignal(BDMAction_Ready);

   vector<string> walletRegIDs;

   auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
   walletRegIDs.push_back(
      wallet1.registerAddresses(scrAddrVec, false));

   //wait on registration ack
   pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

   //get wallets delegate
   auto del1_prom = make_shared<promise<AsyncClient::LedgerDelegate>>();
   auto del1_fut = del1_prom->get_future();
   auto del1_get = [del1_prom](
      ReturnMessage<AsyncClient::LedgerDelegate> delegate)->void
   {
      del1_prom->set_value(move(delegate.get()));
   };
   bdvObj->getLedgerDelegateForWallets(del1_get);
   auto&& main_delegate = del1_fut.get();

   auto ledger_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger_fut = ledger_prom->get_future();
   auto ledger_get =
      [ledger_prom](
         ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger_get);
   auto&& main_ledger = ledger_fut.get();

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 2);

   EXPECT_EQ(main_ledger[0].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[0].getIndex(), 0);

   EXPECT_EQ(main_ledger[1].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[1].getIndex(), 0);

   //add the 2 zc

   auto&& ZC2 = TestUtils::getTx(2, 2); //block 2, tx 2
   auto&& ZChash2 = BtcUtils::getHash256(ZC2);

   vector<BinaryData> zcVec = {ZC1, ZC2};
   auto broadcastId1 = bdvObj->broadcastZC(zcVec);
   
   {
      set<BinaryData> zcHashes = { ZChash1, ZChash2 };
      set<BinaryData> scrAddrSet;

      Tx zctx1(ZC1);
      for (unsigned i = 0; i < zctx1.getNumTxOut(); i++)
         scrAddrSet.insert(zctx1.getScrAddrForTxOut(i));

      Tx zctx2(ZC2);
      for (unsigned i = 0; i < zctx2.getNumTxOut(); i++)
         scrAddrSet.insert(zctx2.getScrAddrForTxOut(i));

      pCallback->waitOnZc(zcHashes, scrAddrSet, broadcastId1);
   }

   //get the new ledgers
   auto ledger2_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger2_fut = ledger2_prom->get_future();
   auto ledger2_get =
      [ledger2_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger2_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger2_get);
   main_ledger = move(ledger2_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 4);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[0].getIndex(), 3);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), UINT32_MAX);
   EXPECT_EQ(main_ledger[1].getIndex(), 2);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   //tx cache testing
   //grab ZC1 from async client
   auto zc_prom1 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut1 = zc_prom1->get_future();
   auto zc_get1 =
      [zc_prom1](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom1->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get1);
   auto zc_obj1 = zc_fut1.get();
   EXPECT_EQ(ZChash1, zc_obj1->getThisHash());
   EXPECT_EQ(zc_obj1->getTxHeight(), UINT32_MAX);

   //grab both zc from async client
   auto zc_prom2 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut2 = zc_prom2->get_future();
   auto zc_get2 =
      [zc_prom2](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom2->set_value(move(txVec));
   };

   set<BinaryData> bothZC = { ZChash1, ZChash2 };
   bdvObj->getTxBatchByHash(bothZC, zc_get2);
   auto zc_obj2 = zc_fut2.get();

   ASSERT_EQ(zc_obj2.size(), 2);

   auto iterZc1 = zc_obj2.find(ZChash1);
   ASSERT_NE(iterZc1, zc_obj2.end());
   ASSERT_NE(iterZc1->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc1->second->getThisHash());
   EXPECT_EQ(iterZc1->second->getTxHeight(), UINT32_MAX);

   auto iterZc2 = zc_obj2.find(ZChash2);
   ASSERT_NE(iterZc2, zc_obj2.end());
   ASSERT_NE(iterZc2->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc2->second->getThisHash());
   EXPECT_EQ(iterZc2->second->getTxHeight(), UINT32_MAX);

   //push an extra block
   TestUtils::appendBlocks({ "2" }, blk0dat_);
   DBTestUtils::triggerNewBlockNotification(theBDMt_);
   pCallback->waitOnSignal(BDMAction_NewBlock);

   //get the new ledgers
   auto ledger3_prom =
      make_shared<promise<vector<::ClientClasses::LedgerEntry>>>();
   auto ledger3_fut = ledger3_prom->get_future();
   auto ledger3_get =
      [ledger3_prom](ReturnMessage<vector<::ClientClasses::LedgerEntry>> ledgerV)->void
   {
      ledger3_prom->set_value(move(ledgerV.get()));
   };
   main_delegate.getHistoryPage(0, ledger3_get);
   main_ledger = move(ledger3_fut.get());

   //check ledgers
   EXPECT_EQ(main_ledger.size(), 5);

   EXPECT_EQ(main_ledger[0].getValue(), -20 * COIN);
   EXPECT_EQ(main_ledger[0].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[0].getIndex(), 2);

   EXPECT_EQ(main_ledger[1].getValue(), -25 * COIN);
   EXPECT_EQ(main_ledger[1].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[1].getIndex(), 1);

   EXPECT_EQ(main_ledger[2].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[2].getBlockNum(), 2);
   EXPECT_EQ(main_ledger[2].getIndex(), 0);

   EXPECT_EQ(main_ledger[3].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[3].getBlockNum(), 1);
   EXPECT_EQ(main_ledger[3].getIndex(), 0);

   EXPECT_EQ(main_ledger[4].getValue(), 50 * COIN);
   EXPECT_EQ(main_ledger[4].getBlockNum(), 0);
   EXPECT_EQ(main_ledger[4].getIndex(), 0);


   //grab ZC1 from async client
   auto zc_prom3 = make_shared<promise<AsyncClient::TxResult>>();
   auto zc_fut3 = zc_prom3->get_future();
   auto zc_get3 =
      [zc_prom3](ReturnMessage<AsyncClient::TxResult> txObj)->void
   {
      auto&& tx = txObj.get();
      zc_prom3->set_value(move(tx));
   };

   bdvObj->getTxByHash(ZChash1, zc_get3);
   auto zc_obj3 = zc_fut3.get();
   EXPECT_EQ(ZChash1, zc_obj3->getThisHash());
   EXPECT_EQ(zc_obj3->getTxHeight(), 2);

   //grab both zc from async client
   auto zc_prom4 = make_shared<promise<AsyncClient::TxBatchResult>>();
   auto zc_fut4 = zc_prom4->get_future();
   auto zc_get4 =
      [zc_prom4](ReturnMessage<AsyncClient::TxBatchResult> txObj)->void
   {
      auto&& txVec = txObj.get();
      zc_prom4->set_value(move(txVec));
   };

   bdvObj->getTxBatchByHash(bothZC, zc_get4);
   auto zc_obj4 = zc_fut4.get();

   ASSERT_EQ(zc_obj4.size(), 2);

   auto iterZc3 = zc_obj4.find(ZChash1);
   ASSERT_NE(iterZc3, zc_obj4.end());
   ASSERT_NE(iterZc3->second, nullptr);
   EXPECT_EQ(ZChash1, iterZc3->second->getThisHash());
   EXPECT_EQ(iterZc3->second->getTxHeight(), 2);

   auto iterZc4 = zc_obj4.find(ZChash2);
   ASSERT_NE(iterZc4, zc_obj4.end());
   ASSERT_NE(iterZc4->second, nullptr);
   EXPECT_EQ(ZChash2, iterZc4->second->getThisHash());
   EXPECT_EQ(iterZc4->second->getTxHeight(), 2);

   //disconnect
   bdvObj->unregisterFromDB();

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_ZcUpdate_RBFLowFee)
{
   //instantiate resolver feed overloaded object
   auto feed = make_shared<ResolverUtils::TestResolverFeed>();

   auto addToFeed = [feed](const BinaryData& key)->void
   {
      auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
      feed->h160ToPubKey_.insert(datapair);
      feed->pubKeyToPrivKey_[datapair.second] = key;
   };

   addToFeed(TestChain::privKeyAddrB);
   addToFeed(TestChain::privKeyAddrC);
   addToFeed(TestChain::privKeyAddrD);
   addToFeed(TestChain::privKeyAddrE);
   addToFeed(TestChain::privKeyAddrF);

   //public server
   startupBIP150CTX(4, true);

   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);
   scrAddrVec.push_back(TestChain::scrAddrD);
   scrAddrVec.push_back(TestChain::scrAddrE);
   scrAddrVec.push_back(TestChain::scrAddrF);

   theBDMt_->start(config.initMode_);

   auto pCallback = make_shared<DBTestUtils::UTCallback>();
   auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
   bdvObj->addPublicKey(serverPubkey);
   bdvObj->connectToRemote();
   bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

   auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
   vector<string> walletRegIDs;
   walletRegIDs.push_back(
      wallet1.registerAddresses(scrAddrVec, false));

   //wait on registration ack
   pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

   //go online
   bdvObj->goOnline();
   pCallback->waitOnSignal(BDMAction_Ready);

   //create tx from utxo lambda
   auto makeTxFromUtxo = [feed](const UTXO& utxo, const BinaryData& recipient)->BinaryData
   {
      auto spender = make_shared<ScriptSpender>(utxo);
      spender->setSequence(0xFFFFFFFF - 2); //flag rbf

      auto recPtr = make_shared<Recipient_P2PKH>(
         recipient.getSliceCopy(1, 20), utxo.getValue());

      Signer signer;
      signer.setFeed(feed);
      signer.addSpender(spender);
      signer.addRecipient(recPtr);

      signer.sign();
      return signer.serializeSignedTx();
   };

   //grab utxo from db
   auto getUtxo = [bdvObj](const BinaryData& addr)->vector<UTXO>
   {
      auto promPtr = make_shared<promise<vector<UTXO>>>();
      auto fut = promPtr->get_future();
      auto getUtxoLbd = [promPtr](ReturnMessage<vector<UTXO>> batch)->void
      {
         promPtr->set_value(batch.get());
      };

      bdvObj->getUTXOsForAddress(addr, false, getUtxoLbd);
      return fut.get();
   };

   //create tx from spender address lambda
   auto makeTx = [makeTxFromUtxo, getUtxo, bdvObj](
      const BinaryData& payer, const BinaryData& recipient)->BinaryData
   {
      auto utxoVec = getUtxo(payer);
      if (utxoVec.size() == 0)
         throw runtime_error("unexpected utxo vec size");

      auto& utxo = utxoVec[0];
      return makeTxFromUtxo(utxo, recipient);
   };

   //grab utxo from raw tx lambda
   auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
   {
      Tx tx(rawTx);
      if (id > tx.getNumTxOut())
         throw runtime_error("invalid txout count");

      auto&& txOut = tx.getTxOutCopy(id);
      
      UTXO utxo;
      utxo.unserializeRaw(txOut.serialize());
      utxo.txOutIndex_ = id;
      utxo.txHash_ = tx.getThisHash();

      return utxo;
   };

   vector<string> walletIDs;
   walletIDs.push_back(wallet1.walletID());

   //grab combined balances lambda
   auto getBalances = [bdvObj, walletIDs](void)->CombinedBalances
   {
      auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
      auto fut = promPtr->get_future();
      auto balLbd = [promPtr](
         ReturnMessage<map<string, CombinedBalances>> combBal)->void
      {
         promPtr->set_value(combBal.get());
      };

      bdvObj->getCombinedBalances(walletIDs, balLbd);
      auto&& balMap = fut.get();

      if (balMap.size() != 1)
         throw runtime_error("unexpected balance map size");

      return balMap.begin()->second;
   };

   //check original balances
   {
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
      ASSERT_NE(iterA, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterA->second.size(), 3);
      EXPECT_EQ(iterA->second[0], 50 * COIN);

      auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
      ASSERT_NE(iterB, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterB->second.size(), 3);
      EXPECT_EQ(iterB->second[0], 70 * COIN);

      auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
      ASSERT_NE(iterC, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterC->second.size(), 3);
      EXPECT_EQ(iterC->second[0], 20 * COIN);

      auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
      ASSERT_NE(iterD, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterD->second.size(), 3);
      EXPECT_EQ(iterD->second[0], 65 * COIN);

      auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
      ASSERT_NE(iterE, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterE->second.size(), 3);
      EXPECT_EQ(iterE->second[0], 30 * COIN);

      auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
      ASSERT_NE(iterF, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterF->second.size(), 3);
      EXPECT_EQ(iterF->second[0], 5 * COIN);
   }

   BinaryData branchPointBlockHash, mainBranchBlockHash;
   {
      auto top = theBDMt_->bdm()->blockchain()->top();
      branchPointBlockHash = top->getThisHash();
   }

   BinaryData bd_BtoC;
   UTXO utxoF;
   {
      //tx from B to C
      bd_BtoC = makeTx(TestChain::scrAddrB, TestChain::scrAddrC);

      //tx from F to A
      auto&& utxoVec = getUtxo(TestChain::scrAddrF);
      ASSERT_EQ(utxoVec.size(), 1);
      utxoF = utxoVec[0];
      auto bd_FtoD = makeTxFromUtxo(utxoF, TestChain::scrAddrA);

      //broadcast
      auto broadcastId1 = bdvObj->broadcastZC(bd_BtoC);
      auto broadcastId2 = bdvObj->broadcastZC(bd_FtoD);

      set<BinaryData> scrAddrSet1, scrAddrSet2;
      
      {
         Tx tx1(bd_BtoC);
         
         Tx tx2(bd_FtoD);
         
         scrAddrSet1.insert(TestChain::scrAddrB);
         scrAddrSet1.insert(TestChain::scrAddrC);

         scrAddrSet2.insert(TestChain::scrAddrF);
         scrAddrSet2.insert(TestChain::scrAddrA);

         pCallback->waitOnZc({tx1.getThisHash()}, scrAddrSet1, broadcastId1);
         pCallback->waitOnZc({tx2.getThisHash()}, scrAddrSet2, broadcastId2);
      }

      //tx from B to A, should fail with RBF low fee
      auto bd_BtoA = makeTx(TestChain::scrAddrB, TestChain::scrAddrA);
      Tx tx(bd_BtoA);

      auto broadcastId3 = bdvObj->broadcastZC(bd_BtoA);
      pCallback->waitOnError(tx.getThisHash(), 
         ArmoryErrorCodes::P2PReject_InsufficientFee, broadcastId3);
 
      //mine
      DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrA, 1);
      pCallback->waitOnSignal(BDMAction_NewBlock);

      //zc C to E
      auto&& utxo = getUtxoFromRawTx(bd_BtoC, 0);
      auto bd_CtoE = makeTxFromUtxo(utxo, TestChain::scrAddrE);
      
      //broadcast
      bdvObj->broadcastZC(bd_CtoE);
      pCallback->waitOnSignal(BDMAction_ZC);

      //mine
      DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrA, 1);
      pCallback->waitOnSignal(BDMAction_NewBlock);

      //check balances
      auto&& combineBalances = getBalances();

      /*
      D doesn't change so there should only be 5 balance entries
      C value does not change but the address sees a ZC in and a
      ZC out so the internal value change tracker counter was 
      incremented, resulting in an entry.
      */
      EXPECT_EQ(combineBalances.addressBalances_.size(), 5);

      auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
      ASSERT_NE(iterA, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterA->second.size(), 3);
      EXPECT_EQ(iterA->second[0], 155 * COIN);

      auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
      ASSERT_NE(iterB, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterB->second.size(), 3);
      EXPECT_EQ(iterB->second[0], 20 * COIN);

      auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
      ASSERT_NE(iterC, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterC->second.size(), 3);
      EXPECT_EQ(iterC->second[0], 20 * COIN);

      auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
      ASSERT_NE(iterE, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterE->second.size(), 3);
      EXPECT_EQ(iterE->second[0], 80 * COIN);

      auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
      ASSERT_NE(iterF, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterF->second.size(), 3);
      EXPECT_EQ(iterF->second[0], 0 * COIN);
   }

   //cleanup
   bdvObj->unregisterFromDB();

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;   
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_ManyLargeWallets)
{
   //public server
   startupBIP150CTX(4, true);

   //
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   auto createNAddresses = [](unsigned count)->vector<BinaryData>
   {
      vector<BinaryData> result;

      for (unsigned i = 0; i < count; i++)
      {
         BinaryWriter bw;
         bw.put_uint8_t(SCRIPT_PREFIX_HASH160);

         auto&& addrData = CryptoPRNG::generateRandom(20);
         bw.put_BinaryData(addrData);

         result.push_back(bw.getData());
      }

      return result;
   };

   auto&& _scrAddrVec1 = createNAddresses(2000);
   _scrAddrVec1.push_back(TestChain::scrAddrA);

   auto&& _scrAddrVec2 = createNAddresses(3);

   auto&& _scrAddrVec3 = createNAddresses(1500);
   _scrAddrVec3.push_back(TestChain::scrAddrB);

   auto&& _scrAddrVec4 = createNAddresses(4);

   auto&& _scrAddrVec5 = createNAddresses(4000);
   _scrAddrVec5.push_back(TestChain::scrAddrC);

   auto&& _scrAddrVec6 = createNAddresses(2);

   auto&& _scrAddrVec7 = createNAddresses(4000);
   _scrAddrVec7.push_back(TestChain::scrAddrE);

   auto&& _scrAddrVec8 = createNAddresses(2);

   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      auto&& wallet2 = bdvObj->instantiateWallet("wallet2");
      walletRegIDs.push_back(
         wallet2.registerAddresses(_scrAddrVec2, false));

      auto&& wallet3 = bdvObj->instantiateWallet("wallet3");
      walletRegIDs.push_back(
         wallet3.registerAddresses(_scrAddrVec3, false));

      auto&& wallet4 = bdvObj->instantiateWallet("wallet4");
      walletRegIDs.push_back(
         wallet4.registerAddresses(_scrAddrVec4, false));

      auto&& wallet5 = bdvObj->instantiateWallet("wallet5");
      walletRegIDs.push_back(
         wallet5.registerAddresses(_scrAddrVec5, false));

      auto&& wallet6 = bdvObj->instantiateWallet("wallet6");
      walletRegIDs.push_back(
         wallet6.registerAddresses(_scrAddrVec6, false));

      auto&& wallet7 = bdvObj->instantiateWallet("wallet7");
      walletRegIDs.push_back(
         wallet7.registerAddresses(_scrAddrVec7, false));

      auto&& wallet8 = bdvObj->instantiateWallet("wallet8");
      walletRegIDs.push_back(
         wallet8.registerAddresses(_scrAddrVec8, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);
      bdvObj->unregisterFromDB();
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_AddrOpLoop)
{
   auto feed = make_shared<ResolverUtils::TestResolverFeed>();
   auto addToFeed = [feed](const BinaryData& key)->void
   {
      auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
      feed->h160ToPubKey_.insert(datapair);
      feed->pubKeyToPrivKey_[datapair.second] = key;
   };

   addToFeed(TestChain::privKeyAddrB);

   //public server
   startupBIP150CTX(4, true);

   //
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   auto createNAddresses = [](unsigned count)->vector<BinaryData>
   {
      vector<BinaryData> result;

      for (unsigned i = 0; i < count; i++)
      {
         BinaryWriter bw;
         bw.put_uint8_t(SCRIPT_PREFIX_HASH160);

         auto&& addrData = CryptoPRNG::generateRandom(20);
         bw.put_BinaryData(addrData);

         result.push_back(bw.getData());
      }

      return result;
   };

   auto&& _scrAddrVec1 = createNAddresses(20);
   _scrAddrVec1.push_back(TestChain::scrAddrA);
   _scrAddrVec1.push_back(TestChain::scrAddrB);
   _scrAddrVec1.push_back(TestChain::scrAddrC);
   _scrAddrVec1.push_back(TestChain::scrAddrD);
   _scrAddrVec1.push_back(TestChain::scrAddrE);
   _scrAddrVec1.push_back(TestChain::scrAddrF);

   set<BinaryData> scrAddrSet;
   scrAddrSet.insert(_scrAddrVec1.begin(), _scrAddrVec1.end());

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //mine
      DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrB, 1000);
      pCallback->waitOnSignal(BDMAction_NewBlock);

      //get utxos
      auto utxo_prom = make_shared<promise<vector<UTXO>>>();
      auto utxo_fut = utxo_prom->get_future();
      auto utxo_get = [utxo_prom](ReturnMessage<vector<UTXO>> utxoV)->void
      {
         utxo_prom->set_value(move(utxoV.get()));
      };
      wallet1.getSpendableTxOutListForValue(UINT64_MAX, utxo_get);
      auto&& utxos = utxo_fut.get();

      DBTestUtils::ZcVector zcVec;

      //get utxo
      unsigned loopCount = 10;
      unsigned stagger = 0;
      for (auto& utxo : utxos)
      {
         if (utxo.getRecipientScrAddr() != TestChain::scrAddrB ||
            utxo.getScript().getSize() != 25 ||
            utxo.getValue() != 50 * COIN)
            continue;

         //sign
         {
            auto spenderA = make_shared<ScriptSpender>(utxo);
            Signer signer;
            signer.addSpender(spenderA);

            auto id = stagger % _scrAddrVec1.size();

            auto recipient = std::make_shared<Recipient_P2PKH>(
               _scrAddrVec1[id].getSliceCopy(1, 20), utxo.getValue());
            signer.addRecipient(recipient);

            signer.setFeed(feed);
            signer.sign();
            auto rawTx = signer.serializeSignedTx();
            zcVec.push_back(signer.serializeSignedTx(), 130000000, stagger++);
         }

         if (stagger < loopCount)
            continue;

         break;
      }

      DBTestUtils::pushNewZc(theBDMt_, zcVec);
      pCallback->waitOnSignal(BDMAction_ZC);

      auto getAddrOp = [bdvObj, &scrAddrSet](
         unsigned heightOffset, unsigned zcOffset)->OutpointBatch
      {
         auto promPtr = make_shared<promise<OutpointBatch>>();
         auto fut = promPtr->get_future();
         auto addrOpLbd = [promPtr](ReturnMessage<OutpointBatch> batch)->void
         {
            promPtr->set_value(batch.get());
         };

         bdvObj->getOutpointsForAddresses(scrAddrSet, heightOffset, zcOffset, addrOpLbd);
         return fut.get();
      };

      auto computeBalance = [](const vector<OutpointData>& data)->uint64_t
      {
         uint64_t total = 0;
         for(auto& op : data)
         { 
            if (op.isSpent_)
               continue;

            total += op.value_;
         }

         return total;
      };

      //check current mined output state
      unsigned heightOffset = 0;
      auto&& addrOp = getAddrOp(heightOffset, UINT32_MAX);
      heightOffset = addrOp.heightCutoff_ + 1;
      ASSERT_EQ(addrOp.outpoints_.size(), 6);
      
      auto iterAddrA = addrOp.outpoints_.find(TestChain::scrAddrA);
      EXPECT_NE(iterAddrA, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrA->second.size(), 1);
      EXPECT_EQ(computeBalance(iterAddrA->second), 50 * COIN);

      auto iterAddrB = addrOp.outpoints_.find(TestChain::scrAddrB);
      EXPECT_NE(iterAddrB, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrB->second.size(), 1007);
      EXPECT_EQ(computeBalance(iterAddrB->second), 50070 * COIN);

      auto iterAddrC = addrOp.outpoints_.find(TestChain::scrAddrC);
      EXPECT_NE(iterAddrC, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrC->second.size(), 4);
      EXPECT_EQ(computeBalance(iterAddrC->second), 20 * COIN);

      auto iterAddrD = addrOp.outpoints_.find(TestChain::scrAddrD);
      EXPECT_NE(iterAddrD, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrD->second.size(), 4);
      EXPECT_EQ(computeBalance(iterAddrD->second), 65 * COIN);

      auto iterAddrE = addrOp.outpoints_.find(TestChain::scrAddrE);
      EXPECT_NE(iterAddrE, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrE->second.size(), 2);
      EXPECT_EQ(computeBalance(iterAddrE->second), 30 * COIN);

      auto iterAddrF = addrOp.outpoints_.find(TestChain::scrAddrF);
      EXPECT_NE(iterAddrF, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrF->second.size(), 4);
      EXPECT_EQ(computeBalance(iterAddrF->second), 5 * COIN);

      //check zc outputs
      auto zcAddrOp = getAddrOp(UINT32_MAX, 0);
      ASSERT_EQ(zcAddrOp.outpoints_.size(), loopCount + 1);

      auto iterZcB = zcAddrOp.outpoints_.find(TestChain::scrAddrB);
      ASSERT_NE(iterZcB, zcAddrOp.outpoints_.end());
      EXPECT_EQ(iterZcB->second.size(), 10);

      for (auto& opB : iterZcB->second)
      {
         EXPECT_EQ(opB.value_, 50 * COIN);
         EXPECT_EQ(opB.txIndex_, 0);
         EXPECT_TRUE(opB.isSpent_);
      }

      for (unsigned z = 0; z < loopCount; z++)
      {
         auto id = z % _scrAddrVec1.size();
         auto& addr = _scrAddrVec1[id];
         
         auto addrIter = zcAddrOp.outpoints_.find(addr);
         ASSERT_NE(addrIter, zcAddrOp.outpoints_.end());
         EXPECT_EQ(addrIter->second.size(), 1);

         auto& op = addrIter->second[0];
         EXPECT_EQ(op.value_, 50 * COIN);
         EXPECT_EQ(op.txHeight_, UINT32_MAX);
      }

      //mine the zc
      for (unsigned z = 0; z < loopCount; z++)
      {
         //mine
         DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrA, 1);
         pCallback->waitOnSignal(BDMAction_NewBlock);

         //grab addrop
         auto&& addr_op = getAddrOp(heightOffset, UINT32_MAX);
         EXPECT_EQ(addr_op.outpoints_.size(), 3);

         //new coinbase to A
         auto iterA = addr_op.outpoints_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, addr_op.outpoints_.end());
         EXPECT_EQ(iterA->second.size(), 1);

         auto& opA = *iterA->second.begin();
         EXPECT_EQ(opA.txIndex_, 0);
         EXPECT_EQ(opA.txOutIndex_, 0);
         EXPECT_EQ(opA.value_, 50 * COIN);
         EXPECT_FALSE(opA.isSpent_);

         //B coinbase input
         auto iterB = addr_op.outpoints_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, addr_op.outpoints_.end());
         EXPECT_EQ(iterB->second.size(), 1);

         auto& opB = *iterB->second.begin();
         EXPECT_EQ(opB.txIndex_, 0);
         EXPECT_EQ(opB.txOutIndex_, 0);
         EXPECT_EQ(opB.value_, 50 * COIN);
         EXPECT_TRUE(opB.isSpent_);
         
         //to recipient
         auto id = z % _scrAddrVec1.size();
         auto& recAddr = _scrAddrVec1[id];
         auto iterR = addr_op.outpoints_.find(recAddr);
         ASSERT_NE(iterR, addr_op.outpoints_.end());
         EXPECT_EQ(iterR->second.size(), 1);

         auto& opR = *iterR->second.begin();
         EXPECT_EQ(opR.txIndex_, 1);
         EXPECT_EQ(opR.value_, 50 * COIN);

         //update cutoff
         heightOffset = addr_op.heightCutoff_ + 1;
      }
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_CombinedCalls)
{
   //public server
   startupBIP150CTX(4, true);

   //
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   auto createNAddresses = [](unsigned count)->vector<BinaryData>
   {
      vector<BinaryData> result;

      for (unsigned i = 0; i < count; i++)
      {
         BinaryWriter bw;
         bw.put_uint8_t(SCRIPT_PREFIX_HASH160);

         auto&& addrData = CryptoPRNG::generateRandom(20);
         bw.put_BinaryData(addrData);

         result.push_back(bw.getData());
      }

      return result;
   };

   auto&& _scrAddrVec1 = createNAddresses(20);
   _scrAddrVec1.push_back(TestChain::scrAddrA);

   auto&& _scrAddrVec2 = createNAddresses(15);
   _scrAddrVec2.push_back(TestChain::scrAddrB);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      auto&& wallet2 = bdvObj->instantiateWallet("wallet2");
      walletRegIDs.push_back(
         wallet2.registerAddresses(_scrAddrVec2, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //balances
      vector<string> walletIDs;
      walletIDs.push_back(wallet1.walletID());
      walletIDs.push_back(wallet2.walletID());

      auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
      auto fut = promPtr->get_future();
      auto balLbd = [promPtr](
         ReturnMessage<map<string, CombinedBalances>> combBal)->void
      {
         promPtr->set_value(combBal.get());
      };

      bdvObj->getCombinedBalances(walletIDs, balLbd);
      auto&& balMap = fut.get();
      ASSERT_EQ(balMap.size(), 2);

      //wallet1
      auto iter1 = balMap.find(walletIDs[0]);
      ASSERT_NE(iter1, balMap.end());

      //sizes
      ASSERT_EQ(iter1->second.walletBalanceAndCount_.size(), 4);
      ASSERT_EQ(iter1->second.addressBalances_.size(), 1);

      //wallet balance
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[0], 50 * COIN);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[1], 0);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[2], 50 * COIN);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[3], 1);

      //scrAddrA balance
      auto addrIter1 = iter1->second.addressBalances_.find(TestChain::scrAddrA);
      ASSERT_NE(addrIter1, iter1->second.addressBalances_.end());
      ASSERT_EQ(addrIter1->second.size(), 3);
      EXPECT_EQ(addrIter1->second[0], 50 * COIN);
      EXPECT_EQ(addrIter1->second[1], 0);
      EXPECT_EQ(addrIter1->second[2], 50 * COIN);

      //wallet2
      auto iter2 = balMap.find(walletIDs[1]);
      ASSERT_NE(iter2, balMap.end());

      //sizes
      ASSERT_EQ(iter2->second.walletBalanceAndCount_.size(), 4);
      ASSERT_EQ(iter2->second.addressBalances_.size(), 1);

      //wallet balance
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[0], 70 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[1], 20 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[2], 70 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[3], 12);

      //scrAddrB balance
      auto addrIter2 = iter2->second.addressBalances_.find(TestChain::scrAddrB);
      ASSERT_NE(addrIter2, iter2->second.addressBalances_.end());
      ASSERT_EQ(addrIter2->second.size(), 3);
      EXPECT_EQ(addrIter2->second[0], 70 * COIN);
      EXPECT_EQ(addrIter2->second[1], 20 * COIN);
      EXPECT_EQ(addrIter2->second[2], 70 * COIN);

      //addr txn counts
      auto promPtr2 = make_shared<promise<map<string, CombinedCounts>>>();
      auto fut2 = promPtr2->get_future();
      auto countLbd = [promPtr2](
         ReturnMessage<map<string, CombinedCounts>> combCount)->void
      {
         promPtr2->set_value(combCount.get());
      };

      bdvObj->getCombinedAddrTxnCounts(walletIDs, countLbd);
      auto&& countMap = fut2.get();
      ASSERT_EQ(countMap.size(), 2);

      //wallet1
      auto iter3 = countMap.find(walletIDs[0]);
      ASSERT_NE(iter3, countMap.end());
      ASSERT_EQ(iter3->second.addressTxnCounts_.size(), 1);

      auto addrIter3 = iter3->second.addressTxnCounts_.find(TestChain::scrAddrA);
      ASSERT_NE(addrIter3, iter3->second.addressTxnCounts_.end());
      EXPECT_EQ(addrIter3->second, 1);

      //wallet2
      auto iter4 = countMap.find(walletIDs[1]);
      ASSERT_NE(iter4, countMap.end());
      ASSERT_EQ(iter4->second.addressTxnCounts_.size(), 1);

      auto addrIter4 = iter4->second.addressTxnCounts_.find(TestChain::scrAddrB);
      ASSERT_NE(addrIter4, iter4->second.addressTxnCounts_.end());
      EXPECT_EQ(addrIter4->second, 12);

      //utxos
      auto promPtr3 = make_shared<promise<vector<UTXO>>>();
      auto fut3 = promPtr3->get_future();
      auto utxoLbd = [promPtr3](ReturnMessage<vector<UTXO>> combUtxo)->void
      {
         promPtr3->set_value(combUtxo.get());
      };

      bdvObj->getCombinedSpendableTxOutListForValue(
         walletIDs, UINT64_MAX, utxoLbd);
      auto&& utxoVec = fut3.get();
      ASSERT_EQ(utxoVec.size(), 1);

      auto& utxo1 = utxoVec[0];
      EXPECT_EQ(utxo1.getValue(), 20 * COIN);
      EXPECT_EQ(utxo1.getRecipientScrAddr(), TestChain::scrAddrB);

      //done
      bdvObj->unregisterFromDB();
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_UnregisterAddresses)
{
   //public server
   startupBIP150CTX(4, true);

   //
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   auto createNAddresses = [](unsigned count)->vector<BinaryData>
   {
      vector<BinaryData> result;

      for (unsigned i = 0; i < count; i++)
      {
         BinaryWriter bw;
         bw.put_uint8_t(SCRIPT_PREFIX_HASH160);

         auto&& addrData = CryptoPRNG::generateRandom(20);
         bw.put_BinaryData(addrData);

         result.push_back(bw.getData());
      }

      return result;
   };

   auto&& _scrAddrVec1 = createNAddresses(20);
   _scrAddrVec1.push_back(TestChain::scrAddrA);

   auto&& _scrAddrVec2 = createNAddresses(15);
   _scrAddrVec2.push_back(TestChain::scrAddrB);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      auto&& wallet2 = bdvObj->instantiateWallet("wallet2");
      walletRegIDs.push_back(
         wallet2.registerAddresses(_scrAddrVec2, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //balances
      vector<string> walletIDs;
      walletIDs.push_back(wallet1.walletID());
      walletIDs.push_back(wallet2.walletID());

      auto getCombinedBalances = [bdvObj](vector<string> walletIDs)->map<string, CombinedBalances>
      {
         auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
         auto fut = promPtr->get_future();
         auto balLbd = [promPtr](
            ReturnMessage<map<string, CombinedBalances>> combBal)->void
         {
            promPtr->set_value(combBal.get());
         };

         bdvObj->getCombinedBalances(walletIDs, balLbd);
         return fut.get();
      };

      auto&& balMap = getCombinedBalances(walletIDs);
      ASSERT_EQ(balMap.size(), 2);

      //wallet1
      auto iter1 = balMap.find(walletIDs[0]);
      ASSERT_NE(iter1, balMap.end());

      //sizes
      ASSERT_EQ(iter1->second.walletBalanceAndCount_.size(), 4);
      ASSERT_EQ(iter1->second.addressBalances_.size(), 1);

      //wallet balance
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[0], 50 * COIN);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[1], 0);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[2], 50 * COIN);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[3], 1);

      //scrAddrA balance
      auto addrIter1 = iter1->second.addressBalances_.find(TestChain::scrAddrA);
      ASSERT_NE(addrIter1, iter1->second.addressBalances_.end());
      ASSERT_EQ(addrIter1->second.size(), 3);
      EXPECT_EQ(addrIter1->second[0], 50 * COIN);
      EXPECT_EQ(addrIter1->second[1], 0);
      EXPECT_EQ(addrIter1->second[2], 50 * COIN);

      //wallet2
      auto iter2 = balMap.find(walletIDs[1]);
      ASSERT_NE(iter2, balMap.end());

      //sizes
      ASSERT_EQ(iter2->second.walletBalanceAndCount_.size(), 4);
      ASSERT_EQ(iter2->second.addressBalances_.size(), 1);

      //wallet balance
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[0], 70 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[1], 20 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[2], 70 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[3], 12);

      //scrAddrB balance
      auto addrIter2 = iter2->second.addressBalances_.find(TestChain::scrAddrB);
      ASSERT_NE(addrIter2, iter2->second.addressBalances_.end());
      ASSERT_EQ(addrIter2->second.size(), 3);
      EXPECT_EQ(addrIter2->second[0], 70 * COIN);
      EXPECT_EQ(addrIter2->second[1], 20 * COIN);
      EXPECT_EQ(addrIter2->second[2], 70 * COIN);

      //addr txn counts
      auto promPtr2 = make_shared<promise<map<string, CombinedCounts>>>();
      auto fut2 = promPtr2->get_future();
      auto countLbd = [promPtr2](
         ReturnMessage<map<string, CombinedCounts>> combCount)->void
      {
         promPtr2->set_value(combCount.get());
      };

      bdvObj->getCombinedAddrTxnCounts(walletIDs, countLbd);
      auto&& countMap = fut2.get();
      ASSERT_EQ(countMap.size(), 2);

      //wallet1
      auto iter3 = countMap.find(walletIDs[0]);
      ASSERT_NE(iter3, countMap.end());
      ASSERT_EQ(iter3->second.addressTxnCounts_.size(), 1);

      auto addrIter3 = iter3->second.addressTxnCounts_.find(TestChain::scrAddrA);
      ASSERT_NE(addrIter3, iter3->second.addressTxnCounts_.end());
      EXPECT_EQ(addrIter3->second, 1);

      //wallet2
      auto iter4 = countMap.find(walletIDs[1]);
      ASSERT_NE(iter4, countMap.end());
      ASSERT_EQ(iter4->second.addressTxnCounts_.size(), 1);

      auto addrIter4 = iter4->second.addressTxnCounts_.find(TestChain::scrAddrB);
      ASSERT_NE(addrIter4, iter4->second.addressTxnCounts_.end());
      EXPECT_EQ(addrIter4->second, 12);

      //utxos
      auto promPtr3 = make_shared<promise<vector<UTXO>>>();
      auto fut3 = promPtr3->get_future();
      auto utxoLbd = [promPtr3](ReturnMessage<vector<UTXO>> combUtxo)->void
      {
         promPtr3->set_value(combUtxo.get());
      };

      bdvObj->getCombinedSpendableTxOutListForValue(
         walletIDs, UINT64_MAX, utxoLbd);
      auto&& utxoVec = fut3.get();
      ASSERT_EQ(utxoVec.size(), 1);

      auto& utxo1 = utxoVec[0];
      EXPECT_EQ(utxo1.getValue(), 20 * COIN);
      EXPECT_EQ(utxo1.getRecipientScrAddr(), TestChain::scrAddrB);

      //mine a couple blocks on the new addresses
      DBTestUtils::mineNewBlock(theBDMt_, _scrAddrVec1[0].getSliceCopy(1, 20), 1);
      pCallback->waitOnSignal(BDMAction_NewBlock);
      
      DBTestUtils::mineNewBlock(theBDMt_, _scrAddrVec1[1].getSliceCopy(1, 20), 1);
      pCallback->waitOnSignal(BDMAction_NewBlock);
      
      DBTestUtils::mineNewBlock(theBDMt_, _scrAddrVec2[0].getSliceCopy(1, 20), 1);
      pCallback->waitOnSignal(BDMAction_NewBlock);
      
      DBTestUtils::mineNewBlock(theBDMt_, _scrAddrVec2[1].getSliceCopy(1, 20), 1);
      pCallback->waitOnSignal(BDMAction_NewBlock);

      //grab balances
      balMap = getCombinedBalances(walletIDs);
      ASSERT_EQ(balMap.size(), 2);

      //wallet1
      iter1 = balMap.find(walletIDs[0]);
      ASSERT_NE(iter1, balMap.end());

      //sizes
      ASSERT_EQ(iter1->second.walletBalanceAndCount_.size(), 4);
      ASSERT_EQ(iter1->second.addressBalances_.size(), 2);

      //wallet balance
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[0], 150 * COIN);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[1], 0);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[2], 150 * COIN);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[3], 3);

      //_scrAddrVec1[0] balance
      addrIter1 = iter1->second.addressBalances_.find(_scrAddrVec1[0]);
      ASSERT_NE(addrIter1, iter1->second.addressBalances_.end());
      ASSERT_EQ(addrIter1->second.size(), 3);
      EXPECT_EQ(addrIter1->second[0], 50 * COIN);
      EXPECT_EQ(addrIter1->second[1], 0);
      EXPECT_EQ(addrIter1->second[2], 50 * COIN);

      //_scrAddrVec1[1] balance
      addrIter1 = iter1->second.addressBalances_.find(_scrAddrVec1[1]);
      ASSERT_NE(addrIter1, iter1->second.addressBalances_.end());
      ASSERT_EQ(addrIter1->second.size(), 3);
      EXPECT_EQ(addrIter1->second[0], 50 * COIN);
      EXPECT_EQ(addrIter1->second[1], 0);
      EXPECT_EQ(addrIter1->second[2], 50 * COIN);

      //wallet2
      iter2 = balMap.find(walletIDs[1]);
      ASSERT_NE(iter2, balMap.end());

      //sizes
      ASSERT_EQ(iter2->second.walletBalanceAndCount_.size(), 4);
      ASSERT_EQ(iter2->second.addressBalances_.size(), 2);

      //wallet balance
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[0], 170 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[1], 20 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[2], 170 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[3], 14);

      //_scrAddrVec2[0] balance
      addrIter2 = iter2->second.addressBalances_.find(_scrAddrVec2[0]);
      ASSERT_NE(addrIter2, iter2->second.addressBalances_.end());
      ASSERT_EQ(addrIter2->second.size(), 3);
      EXPECT_EQ(addrIter2->second[0], 50 * COIN);
      EXPECT_EQ(addrIter2->second[1], 0);
      EXPECT_EQ(addrIter2->second[2], 50 * COIN);

      //_scrAddrVec2[1] balance
      addrIter2 = iter2->second.addressBalances_.find(_scrAddrVec2[1]);
      ASSERT_NE(addrIter2, iter2->second.addressBalances_.end());
      ASSERT_EQ(addrIter2->second.size(), 3);
      EXPECT_EQ(addrIter2->second[0], 50 * COIN);
      EXPECT_EQ(addrIter2->second[1], 0);
      EXPECT_EQ(addrIter2->second[2], 50 * COIN);

      //unregister some addresses
      walletRegIDs.clear();
      walletRegIDs.push_back(
         wallet1.unregisterAddresses({ _scrAddrVec1[0], _scrAddrVec1[5]}));
      walletRegIDs.push_back(
         wallet2.unregisterAddresses({ _scrAddrVec2[1], _scrAddrVec2[6]}));

      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //grab balances again
      balMap = getCombinedBalances(walletIDs);
      ASSERT_EQ(balMap.size(), 2);

      //wallet1
      iter1 = balMap.find(walletIDs[0]);
      ASSERT_NE(iter1, balMap.end());

      //sizes
      ASSERT_EQ(iter1->second.walletBalanceAndCount_.size(), 4);
      ASSERT_EQ(iter1->second.addressBalances_.size(), 0);

      //wallet balance
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[0], 100 * COIN);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[1], 0);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[2], 100 * COIN);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[3], 2);

      //wallet2
      iter2 = balMap.find(walletIDs[1]);
      ASSERT_NE(iter2, balMap.end());

      //sizes
      ASSERT_EQ(iter2->second.walletBalanceAndCount_.size(), 4);
      ASSERT_EQ(iter2->second.addressBalances_.size(), 0);

      //wallet balance
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[0], 120 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[1], 20 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[2], 120 * COIN);
      EXPECT_EQ(iter2->second.walletBalanceAndCount_[3], 13);

      //unregister a wallet
      walletRegIDs.clear();
      walletRegIDs.push_back(wallet2.unregister());
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //grab balances again
      balMap = getCombinedBalances(walletIDs);
      ASSERT_EQ(balMap.size(), 0); //should be 0, as one wallet in walletIDs is invalid

      //grab balances again
      balMap = getCombinedBalances({wallet1id});
      ASSERT_EQ(balMap.size(), 1);

      //mine a block
      DBTestUtils::mineNewBlock(theBDMt_, _scrAddrVec1[2].getSliceCopy(1, 20), 1);
      pCallback->waitOnSignal(BDMAction_NewBlock);

      //grab balances again
      balMap = getCombinedBalances({wallet1id});
      ASSERT_EQ(balMap.size(), 1);
      
      //wallet1
      iter1 = balMap.find(walletIDs[0]);
      ASSERT_NE(iter1, balMap.end());

      //sizes
      ASSERT_EQ(iter1->second.walletBalanceAndCount_.size(), 4);
      ASSERT_EQ(iter1->second.addressBalances_.size(), 1);

      //wallet balance
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[0], 150 * COIN);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[1], 0);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[2], 150 * COIN);
      EXPECT_EQ(iter1->second.walletBalanceAndCount_[3], 3);

      addrIter1 = iter1->second.addressBalances_.find(_scrAddrVec1[2]);
      ASSERT_NE(addrIter1, iter1->second.addressBalances_.end());
      ASSERT_EQ(addrIter1->second.size(), 3);
      EXPECT_EQ(addrIter1->second[0], 50 * COIN);
      EXPECT_EQ(addrIter1->second[1], 0);
      EXPECT_EQ(addrIter1->second[2], 50 * COIN);

      //done
      bdvObj->unregisterFromDB();
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_DynamicReorg)
{
   //instantiate resolver feed overloaded object
   auto feed = make_shared<ResolverUtils::TestResolverFeed>();

   auto addToFeed = [feed](const BinaryData& key)->void
   {
      auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
      feed->h160ToPubKey_.insert(datapair);
      feed->pubKeyToPrivKey_[datapair.second] = key;
   };

   addToFeed(TestChain::privKeyAddrB);
   addToFeed(TestChain::privKeyAddrC);
   addToFeed(TestChain::privKeyAddrD);
   addToFeed(TestChain::privKeyAddrE);
   addToFeed(TestChain::privKeyAddrF);

   //public server
   startupBIP150CTX(4, true);

   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);
   scrAddrVec.push_back(TestChain::scrAddrD);
   scrAddrVec.push_back(TestChain::scrAddrE);
   scrAddrVec.push_back(TestChain::scrAddrF);

   theBDMt_->start(config.initMode_);

   auto pCallback = make_shared<DBTestUtils::UTCallback>();
   auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
   bdvObj->addPublicKey(serverPubkey);
   bdvObj->connectToRemote();
   bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

   auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
   vector<string> walletRegIDs;
   walletRegIDs.push_back(
      wallet1.registerAddresses(scrAddrVec, false));

   //wait on registration ack
   pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

   //go online
   bdvObj->goOnline();
   pCallback->waitOnSignal(BDMAction_Ready);

   //create tx from utxo lambda
   auto makeTxFromUtxo = [feed](const UTXO& utxo, const BinaryData& recipient)->BinaryData
   {
      auto spender = make_shared<ScriptSpender>(utxo);
      auto recPtr = make_shared<Recipient_P2PKH>(recipient.getSliceCopy(1, 20), utxo.getValue());

      Signer signer;
      signer.setFeed(feed);
      signer.addSpender(spender);
      signer.addRecipient(recPtr);

      signer.sign();
      return signer.serializeSignedTx();
   };

   //grab utxo from db
   auto getUtxo = [bdvObj](const BinaryData& addr)->vector<UTXO>
   {
      auto promPtr = make_shared<promise<vector<UTXO>>>();
      auto fut = promPtr->get_future();
      auto getUtxoLbd = [promPtr](ReturnMessage<vector<UTXO>> batch)->void
      {
         promPtr->set_value(batch.get());
      };

      bdvObj->getUTXOsForAddress(addr, false, getUtxoLbd);
      return fut.get();
   };

   //create tx from spender address lambda
   auto makeTx = [makeTxFromUtxo, getUtxo, bdvObj](
      const BinaryData& payer, const BinaryData& recipient)->BinaryData
   {
      auto utxoVec = getUtxo(payer);
      if (utxoVec.size() == 0)
         throw runtime_error("unexpected utxo vec size");

      auto& utxo = utxoVec[0];
      return makeTxFromUtxo(utxo, recipient);
   };

   //grab utxo from raw tx lambda
   auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
   {
      Tx tx(rawTx);
      if (id > tx.getNumTxOut())
         throw runtime_error("invalid txout count");

      auto&& txOut = tx.getTxOutCopy(id);
      
      UTXO utxo;
      utxo.unserializeRaw(txOut.serialize());
      utxo.txOutIndex_ = id;
      utxo.txHash_ = tx.getThisHash();

      return utxo;
   };

   vector<string> walletIDs;
   walletIDs.push_back(wallet1.walletID());

   //grab combined balances lambda
   auto getBalances = [bdvObj, walletIDs](void)->CombinedBalances
   {
      auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
      auto fut = promPtr->get_future();
      auto balLbd = [promPtr](
         ReturnMessage<map<string, CombinedBalances>> combBal)->void
      {
         promPtr->set_value(combBal.get());
      };

      bdvObj->getCombinedBalances(walletIDs, balLbd);
      auto&& balMap = fut.get();

      if (balMap.size() != 1)
         throw runtime_error("unexpected balance map size");

      return balMap.begin()->second;
   };

   //check original balances
   {
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
      ASSERT_NE(iterA, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterA->second.size(), 3);
      EXPECT_EQ(iterA->second[0], 50 * COIN);

      auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
      ASSERT_NE(iterB, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterB->second.size(), 3);
      EXPECT_EQ(iterB->second[0], 70 * COIN);

      auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
      ASSERT_NE(iterC, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterC->second.size(), 3);
      EXPECT_EQ(iterC->second[0], 20 * COIN);

      auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
      ASSERT_NE(iterD, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterD->second.size(), 3);
      EXPECT_EQ(iterD->second[0], 65 * COIN);

      auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
      ASSERT_NE(iterE, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterE->second.size(), 3);
      EXPECT_EQ(iterE->second[0], 30 * COIN);

      auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
      ASSERT_NE(iterF, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterF->second.size(), 3);
      EXPECT_EQ(iterF->second[0], 5 * COIN);
   }

   BinaryData branchPointBlockHash, mainBranchBlockHash;
   {
      auto top = theBDMt_->bdm()->blockchain()->top();
      branchPointBlockHash = top->getThisHash();
   }

   //main branch
   BinaryData bd_BtoC;
   UTXO utxoF;
   {
      //tx from B to C
      bd_BtoC = makeTx(TestChain::scrAddrB, TestChain::scrAddrC);

      //tx from F to A
      auto&& utxoVec = getUtxo(TestChain::scrAddrF);
      ASSERT_EQ(utxoVec.size(), 1);
      utxoF = utxoVec[0];
      auto bd_FtoD = makeTxFromUtxo(utxoF, TestChain::scrAddrA);

      //broadcast
      DBTestUtils::ZcVector zcVec;
      zcVec.push_back(bd_BtoC, 1300000000);
      zcVec.push_back(bd_FtoD, 1300000001);
      DBTestUtils::pushNewZc(theBDMt_, zcVec);
      pCallback->waitOnSignal(BDMAction_ZC);

      //mine
      DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrA, 1);
      pCallback->waitOnSignal(BDMAction_NewBlock);

      //zc C to E
      auto&& utxo = getUtxoFromRawTx(bd_BtoC, 0);
      auto bd_CtoE = makeTxFromUtxo(utxo, TestChain::scrAddrE);
      
      //broadcast
      zcVec.zcVec_.clear();
      zcVec.push_back(bd_CtoE, 1300000002);
      DBTestUtils::pushNewZc(theBDMt_, zcVec);
      pCallback->waitOnSignal(BDMAction_ZC);

      //mine
      DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrA, 1);
      pCallback->waitOnSignal(BDMAction_NewBlock);

      //check balances
      auto&& combineBalances = getBalances();

      /*
      D doesn't change so there should only be 5 balance entries
      C value does not change but the address sees a ZC in and a
      ZC out so the internal value change tracker counter was 
      incremented, resulting in an entry.
      */
      EXPECT_EQ(combineBalances.addressBalances_.size(), 5);

      auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
      ASSERT_NE(iterA, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterA->second.size(), 3);
      EXPECT_EQ(iterA->second[0], 155 * COIN);

      auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
      ASSERT_NE(iterB, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterB->second.size(), 3);
      EXPECT_EQ(iterB->second[0], 20 * COIN);

      auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
      ASSERT_NE(iterC, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterC->second.size(), 3);
      EXPECT_EQ(iterC->second[0], 20 * COIN);

      auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
      ASSERT_NE(iterE, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterE->second.size(), 3);
      EXPECT_EQ(iterE->second[0], 80 * COIN);

      auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
      ASSERT_NE(iterF, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterF->second.size(), 3);
      EXPECT_EQ(iterF->second[0], 0 * COIN);

      {
         auto top = theBDMt_->bdm()->blockchain()->top();
         mainBranchBlockHash = top->getThisHash();
      }
   }

   //reorg
   {
      //set branching point
      DBTestUtils::setReorgBranchingPoint(theBDMt_, branchPointBlockHash);

      //tx from F to D
      auto bd_FtoD = makeTxFromUtxo(utxoF, TestChain::scrAddrD);

      //broadcast
      DBTestUtils::ZcVector zcVec;
      zcVec.push_back(bd_BtoC, 1300000000, 2); //repeat B to C
      zcVec.push_back(bd_FtoD, 1300000001, 2);

      //zc D to E
      auto&& utxo = getUtxoFromRawTx(bd_FtoD, 0);
      auto bd_DtoE = makeTxFromUtxo(utxo, TestChain::scrAddrE);

      //broadcast
      zcVec.push_back(bd_DtoE, 1300000002, 3);

      /*
      Pass true to stage the zc. Cant broadcast that stuff until 
      the fork is live.
      */
      DBTestUtils::pushNewZc(theBDMt_, zcVec, true);

      //mine 3 blocks to outpace original chain
      DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrB, 3);
      EXPECT_EQ(pCallback->waitOnReorg(), 5);

      //wait on ZC now, as the staged transactions have been pushed
      pCallback->waitOnSignal(BDMAction_ZC);

      //check balances
      auto&& combineBalances = getBalances();

      /*
      This triggers a reorg.
      F does not receive any effective change in balance from the
      previous top.

      D value does not change but this is due to a ZC spending the coins
      out, so the internal id is updated.
      */
      EXPECT_EQ(combineBalances.addressBalances_.size(), 5);

      auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
      ASSERT_NE(iterA, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterA->second.size(), 3);
      EXPECT_EQ(iterA->second[0], 50 * COIN);

      auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
      ASSERT_NE(iterB, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterB->second.size(), 3);
      EXPECT_EQ(iterB->second[0], 170 * COIN);

      auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
      ASSERT_NE(iterC, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterC->second.size(), 3);
      EXPECT_EQ(iterC->second[0], 70 * COIN);

      auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
      ASSERT_NE(iterD, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterD->second.size(), 3);
      EXPECT_EQ(iterD->second[0], 65 * COIN);

      auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
      ASSERT_NE(iterE, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterE->second.size(), 3);
      EXPECT_EQ(iterE->second[0], 35 * COIN);
   }

   //back to main chain
   {
      //set branching point
      DBTestUtils::setReorgBranchingPoint(theBDMt_, mainBranchBlockHash);

      //mine 2 blocks to outpace forked chain
      DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrF, 2);
      EXPECT_EQ(pCallback->waitOnReorg(), 5);

      //check balances
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
      ASSERT_NE(iterA, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterA->second.size(), 3);
      EXPECT_EQ(iterA->second[0], 155 * COIN);

      auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
      ASSERT_NE(iterB, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterB->second.size(), 3);
      EXPECT_EQ(iterB->second[0], 20 * COIN);

      auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
      ASSERT_NE(iterC, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterC->second.size(), 3);
      EXPECT_EQ(iterC->second[0], 20 * COIN);

      auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
      ASSERT_NE(iterD, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterD->second.size(), 3);
      EXPECT_EQ(iterD->second[0], 65 * COIN);

      auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
      ASSERT_NE(iterE, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterE->second.size(), 3);
      EXPECT_EQ(iterE->second[0], 80 * COIN);

      auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
      ASSERT_NE(iterF, combineBalances.addressBalances_.end());
      ASSERT_EQ(iterF->second.size(), 3);
      EXPECT_EQ(iterF->second[0], 100 * COIN);
   }

   //disconnect
   bdvObj->unregisterFromDB();

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_GetTxByHash)
{
   //instantiate resolver feed overloaded object
   auto feed = make_shared<ResolverUtils::TestResolverFeed>();

   auto addToFeed = [feed](const BinaryData& key)->void
   {
      auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
      feed->h160ToPubKey_.insert(datapair);
      feed->pubKeyToPrivKey_[datapair.second] = key;
   };

   addToFeed(TestChain::privKeyAddrB);
   addToFeed(TestChain::privKeyAddrC);
   addToFeed(TestChain::privKeyAddrD);
   addToFeed(TestChain::privKeyAddrE);
   addToFeed(TestChain::privKeyAddrF);

   //public server
   startupBIP150CTX(4, true);

   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();

   vector<BinaryData> scrAddrVec;
   scrAddrVec.push_back(TestChain::scrAddrA);
   scrAddrVec.push_back(TestChain::scrAddrB);
   scrAddrVec.push_back(TestChain::scrAddrC);
   scrAddrVec.push_back(TestChain::scrAddrD);
   scrAddrVec.push_back(TestChain::scrAddrE);
   scrAddrVec.push_back(TestChain::scrAddrF);

   theBDMt_->start(config.initMode_);

   auto pCallback = make_shared<DBTestUtils::UTCallback>();
   auto bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
   bdvObj->addPublicKey(serverPubkey);
   bdvObj->connectToRemote();
   bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

   auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
   vector<string> walletRegIDs;
   walletRegIDs.push_back(
      wallet1.registerAddresses(scrAddrVec, false));

   //wait on registration ack
   pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

   //go online
   bdvObj->goOnline();
   pCallback->waitOnSignal(BDMAction_Ready);

   //grab mined tx
   auto&& ZC1 = TestUtils::getTx(5, 2); //block 5, tx 2
   auto&& hash1 = BtcUtils::getHash256(ZC1);
   
   auto getTxLbd = [bdvObj](const BinaryData& hash)->ReturnMessage<AsyncClient::TxResult>
   {
      auto promPtr = make_shared<promise<ReturnMessage<AsyncClient::TxResult>>>();
      auto fut = promPtr->get_future();
      auto lbd = [promPtr](ReturnMessage<AsyncClient::TxResult> txObj)
      {
         promPtr->set_value(txObj);
      };

      bdvObj->getTxByHash(hash, lbd);
      return fut.get();
   };

   //fetch mined tx
   auto&& txObj1 = getTxLbd(hash1);

   //fetch invalid tx
   auto&& txObj2 = getTxLbd(READHEX("000102030405060708090A0B0C0D0E0F000102030405060708090A0B0C0D0E0F"));

   try
   {
      auto&& tx = txObj1.get();
      auto hash = BtcUtils::getHash256(tx->serialize());
      EXPECT_EQ(hash, hash1);
   }
   catch (exception&)
   {
      ASSERT_FALSE(true);
   }  

   try
   {
      auto&& tx = txObj2.get();
      auto hash = BtcUtils::getHash256(tx->serialize());
      ASSERT_FALSE(true);
   }
   catch (ClientMessageError& e)
   {
      EXPECT_EQ(string(e.what()), string("Error processing command: 80\nfailed to grab tx by hash"));
   }
   catch (...)
   {
      ASSERT_FALSE(true);
   }

   //test cache hit
   auto&& txObj3 = getTxLbd(hash1);
   try
   {
      auto&& tx = txObj3.get();
      auto hash = BtcUtils::getHash256(tx->serialize());
      EXPECT_EQ(hash, hash1);
   }
   catch (exception&)
   {
      ASSERT_FALSE(true);
   }  

   //grab a couple utxos
   auto promUtxo = make_shared<promise<vector<UTXO>>>();
   auto futUtxo = promUtxo->get_future();
   auto lbdUtxo = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
   {
      promUtxo->set_value(msg.get());
   };

   wallet1.getSpendableTxOutListForValue(UINT64_MAX, lbdUtxo);
   auto&& utxoVec = futUtxo.get();

   //create 2 zc
   BinaryData rawTx1;
   {
      //5 from E, 3 to A, change to C
      Signer signer;

      auto spender = make_shared<ScriptSpender>(utxoVec[0]);
      signer.addSpender(spender);

      auto recA = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrA.getSliceCopy(1, 20), 3 * COIN);
      signer.addRecipient(recA);

      auto recChange = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrC.getSliceCopy(1, 20), 
         spender->getValue() - recA->getValue());
      signer.addRecipient(recChange);

      signer.setFeed(feed);
      signer.sign();
      rawTx1 = signer.serializeSignedTx();
   }
   
   BinaryData rawTx2;
   {
      //20 from B, 5 to C, change to E
      Signer signer;

      auto spender = make_shared<ScriptSpender>(utxoVec.back());
      signer.addSpender(spender);

      auto recC = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrC.getSliceCopy(1, 20), 5 * COIN);
      signer.addRecipient(recC);

      auto recChange = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrE.getSliceCopy(1, 20), 
         spender->getValue() - recC->getValue());
      signer.addRecipient(recChange);

      signer.setFeed(feed);
      signer.sign();
      rawTx2 = signer.serializeSignedTx();
   }

   //push the 2 zc through the node
   DBTestUtils::ZcVector zcVec;
   zcVec.push_back(rawTx1, 1000000000);
   zcVec.push_back(rawTx2, 1000000001);
   DBTestUtils::pushNewZc(theBDMt_, zcVec);

   //wait on them
   Tx tx1(rawTx1);
   Tx tx2(rawTx2);
   set<BinaryData> zcHashes = {tx1.getThisHash(), tx2.getThisHash()};

   set<BinaryData> zcAddresses;
   zcAddresses.insert(TestChain::scrAddrA);
   zcAddresses.insert(TestChain::scrAddrB);
   zcAddresses.insert(TestChain::scrAddrC);
   zcAddresses.insert(TestChain::scrAddrE);

   pCallback->waitOnZc(zcHashes, zcAddresses, "");

   //grab them
   auto&& txObj4 = getTxLbd(tx1.getThisHash()).get();
   auto&& txObj5 = getTxLbd(tx2.getThisHash()).get();
 
   ASSERT_NE(txObj4, nullptr);
   ASSERT_NE(txObj5, nullptr);

   EXPECT_EQ(txObj4->getThisHash(), tx1.getThisHash());
   EXPECT_EQ(txObj4->getTxHeight(), UINT32_MAX);
   EXPECT_EQ(txObj4->getTxIndex(), 0);

   EXPECT_EQ(txObj5->getThisHash(), tx2.getThisHash());
   EXPECT_EQ(txObj5->getTxHeight(), UINT32_MAX);
   EXPECT_EQ(txObj5->getTxIndex(), 1);

   //create 2 more zc
   BinaryData rawTx3;
   {
      //25 from E, 5 to A, change to C
      Signer signer;

      auto spender = make_shared<ScriptSpender>(utxoVec[1]);
      signer.addSpender(spender);

      auto recA = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
      signer.addRecipient(recA);

      auto recChange = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrC.getSliceCopy(1, 20), 
         spender->getValue() - recA->getValue());
      signer.addRecipient(recChange);

      signer.setFeed(feed);
      signer.sign();
      rawTx3 = signer.serializeSignedTx();
   }
   
   BinaryData rawTx4;
   {
      //5 from D, 4 to C, change to E
      Signer signer;

      auto spender = make_shared<ScriptSpender>(utxoVec[2]);
      signer.addSpender(spender);

      auto recC = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrC.getSliceCopy(1, 20), 4 * COIN);
      signer.addRecipient(recC);

      auto recChange = make_shared<Recipient_P2PKH>(
         TestChain::scrAddrE.getSliceCopy(1, 20), 
         spender->getValue() - recC->getValue());
      signer.addRecipient(recChange);

      signer.setFeed(feed);
      signer.sign();
      rawTx4 = signer.serializeSignedTx();
   }

   //push the 2 zc through the rpc
   auto broadcastId1 = bdvObj->broadcastThroughRPC(rawTx3);
   auto broadcastId2 = bdvObj->broadcastThroughRPC(rawTx4);

   //wait on them
   Tx tx3(rawTx3);
   Tx tx4(rawTx4);
   zcHashes.clear();

   set<BinaryData> zcAddresses1;
   zcAddresses1.insert(TestChain::scrAddrA);
   zcAddresses1.insert(TestChain::scrAddrC);
   zcAddresses1.insert(TestChain::scrAddrE);

   set<BinaryData> zcAddresses2;
   zcAddresses2.insert(TestChain::scrAddrD);
   zcAddresses2.insert(TestChain::scrAddrC);
   zcAddresses2.insert(TestChain::scrAddrE);

   pCallback->waitOnZc({tx3.getThisHash()}, zcAddresses1, broadcastId1);
   pCallback->waitOnZc({tx4.getThisHash()}, zcAddresses2, broadcastId2);

   //grab them
   auto&& txObj6 = getTxLbd(tx3.getThisHash()).get();
   auto&& txObj7 = getTxLbd(tx4.getThisHash()).get();
 
   ASSERT_NE(txObj6, nullptr);
   ASSERT_NE(txObj7, nullptr);

   EXPECT_EQ(txObj6->getThisHash(), tx3.getThisHash());
   EXPECT_EQ(txObj6->getTxHeight(), UINT32_MAX);
   EXPECT_EQ(txObj6->getTxIndex(), 2);

   EXPECT_EQ(txObj7->getThisHash(), tx4.getThisHash());
   EXPECT_EQ(txObj7->getTxHeight(), UINT32_MAX);
   EXPECT_EQ(txObj7->getTxIndex(), 3);

   {
      //try to grab from another bdvobj
      auto pCallback2 = make_shared<DBTestUtils::UTCallback>();
      auto bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback2);
      bdvObj2->addPublicKey(serverPubkey);
      bdvObj2->connectToRemote();
      bdvObj2->registerWithDB(NetworkConfig::getMagicBytes());

      bdvObj2->goOnline();
      pCallback2->waitOnSignal(BDMAction_Ready);

      //
      auto getTxLbd2 = [bdvObj2](const BinaryData& hash)->ReturnMessage<AsyncClient::TxResult>
      {
         auto promPtr = make_shared<promise<ReturnMessage<AsyncClient::TxResult>>>();
         auto fut = promPtr->get_future();
         auto lbd = [promPtr](ReturnMessage<AsyncClient::TxResult> txObj)
         {
            promPtr->set_value(txObj);
         };

         bdvObj2->getTxByHash(hash, lbd);
         return fut.get();
      };

      //grab the zc
      auto&& txObj8 = getTxLbd2(tx1.getThisHash()).get();
      auto&& txObj10 = getTxLbd2(tx3.getThisHash()).get();
   
      ASSERT_NE(txObj8, nullptr);
      ASSERT_NE(txObj10, nullptr);

      EXPECT_EQ(txObj8->getThisHash(), tx1.getThisHash());
      EXPECT_EQ(txObj8->getTxHeight(), UINT32_MAX);
      EXPECT_EQ(txObj8->getTxIndex(), 0);

      EXPECT_EQ(txObj10->getThisHash(), tx3.getThisHash());
      EXPECT_EQ(txObj10->getTxHeight(), UINT32_MAX);
      EXPECT_EQ(txObj10->getTxIndex(), 2);

      //batch fetch partial cache hit
      auto getTxLbd3 = [bdvObj2](const set<BinaryData>& hashes)->
         ReturnMessage<AsyncClient::TxBatchResult>
      {
         auto promPtr = make_shared<promise<ReturnMessage<AsyncClient::TxBatchResult>>>();
         auto fut = promPtr->get_future();
         auto lbd = [promPtr](ReturnMessage<AsyncClient::TxBatchResult> txObj)
         {
            promPtr->set_value(txObj);
         };

         bdvObj2->getTxBatchByHash(hashes, lbd);
         return fut.get();
      };

      zcHashes.clear();
      zcHashes = 
      {
         tx1.getThisHash(), 
         tx2.getThisHash(), 
         tx3.getThisHash(), 
         tx4.getThisHash()
      };

      auto&& txMap = getTxLbd3(zcHashes).get();
      ASSERT_EQ(txMap.size(), 4);

      auto iter = txMap.find(tx1.getThisHash());
      ASSERT_NE(iter, txMap.end());
      EXPECT_EQ(iter->second->getThisHash(), tx1.getThisHash());
      EXPECT_EQ(iter->second->getTxHeight(), UINT32_MAX);
      EXPECT_EQ(iter->second->getTxIndex(), 0);

      iter = txMap.find(tx2.getThisHash());
      ASSERT_NE(iter, txMap.end());
      EXPECT_EQ(iter->second->getThisHash(), tx2.getThisHash());
      EXPECT_EQ(iter->second->getTxHeight(), UINT32_MAX);
      EXPECT_EQ(iter->second->getTxIndex(), 1);

      iter = txMap.find(tx3.getThisHash());
      ASSERT_NE(iter, txMap.end());
      EXPECT_EQ(iter->second->getThisHash(), tx3.getThisHash());
      EXPECT_EQ(iter->second->getTxHeight(), UINT32_MAX);
      EXPECT_EQ(iter->second->getTxIndex(), 2);

      iter = txMap.find(tx4.getThisHash());
      ASSERT_NE(iter, txMap.end());
      EXPECT_EQ(iter->second->getThisHash(), tx4.getThisHash());
      EXPECT_EQ(iter->second->getTxHeight(), UINT32_MAX);
      EXPECT_EQ(iter->second->getTxIndex(), 3);

      //batch fetch full cache hit
      txMap = getTxLbd3(zcHashes).get();
      ASSERT_EQ(txMap.size(), 4);

      iter = txMap.find(tx1.getThisHash());
      ASSERT_NE(iter, txMap.end());
      EXPECT_EQ(iter->second->getThisHash(), tx1.getThisHash());
      EXPECT_EQ(iter->second->getTxHeight(), UINT32_MAX);
      EXPECT_EQ(iter->second->getTxIndex(), 0);

      iter = txMap.find(tx2.getThisHash());
      ASSERT_NE(iter, txMap.end());
      EXPECT_EQ(iter->second->getThisHash(), tx2.getThisHash());
      EXPECT_EQ(iter->second->getTxHeight(), UINT32_MAX);
      EXPECT_EQ(iter->second->getTxIndex(), 1);

      iter = txMap.find(tx3.getThisHash());
      ASSERT_NE(iter, txMap.end());
      EXPECT_EQ(iter->second->getThisHash(), tx3.getThisHash());
      EXPECT_EQ(iter->second->getTxHeight(), UINT32_MAX);
      EXPECT_EQ(iter->second->getTxIndex(), 2);

      iter = txMap.find(tx4.getThisHash());
      ASSERT_NE(iter, txMap.end());
      EXPECT_EQ(iter->second->getThisHash(), tx4.getThisHash());
      EXPECT_EQ(iter->second->getTxHeight(), UINT32_MAX);
      EXPECT_EQ(iter->second->getTxIndex(), 3);

      //batch fetch an empty hash
      {
         auto promPtr = make_shared<promise<ReturnMessage<AsyncClient::TxBatchResult>>>();
         auto fut = promPtr->get_future();
         auto lbd = [promPtr](ReturnMessage<AsyncClient::TxBatchResult> txObj)
         {
            promPtr->set_value(txObj);
         };

         set<BinaryData> hashesEmpty;
         hashesEmpty.insert(BtcUtils::EmptyHash());
         bdvObj2->getTxBatchByHash(hashesEmpty, lbd);
         auto&& reply = fut.get();
      }
      
      //disconnect
      bdvObj2->unregisterFromDB();
   }

   //disconnect
   bdvObj->unregisterFromDB();

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_GetSpentness)
{
   auto feed = make_shared<ResolverUtils::TestResolverFeed>();
   auto addToFeed = [feed](const BinaryData& key)->void
   {
      auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
      feed->h160ToPubKey_.insert(datapair);
      feed->pubKeyToPrivKey_[datapair.second] = key;
   };

   addToFeed(TestChain::privKeyAddrB);

   //public server
   startupBIP150CTX(4, true);

   //
   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   struct KeyPair
   {
      SecureBinaryData priv_;
      SecureBinaryData pub_;
      BinaryData scrHash_;
   };

   vector<KeyPair> keyPairs;
   auto createNAddresses = [&keyPairs](unsigned count)->vector<BinaryData>
   {
      vector<BinaryData> result;
      for (unsigned i = 0; i < count; i++)
      {
         KeyPair kp;
         kp.priv_ = CryptoPRNG::generateRandom(32);
         kp.pub_ = CryptoECDSA().ComputePublicKey(kp.priv_, true);
         kp.scrHash_ = BtcUtils::getHash160(kp.pub_);

         BinaryWriter bw;
         bw.put_uint8_t(SCRIPT_PREFIX_HASH160);
         bw.put_BinaryData(kp.scrHash_);

         result.push_back(bw.getData());
         keyPairs.emplace_back(move(kp));
      }

      return result;
   };

   auto&& _scrAddrVec1 = createNAddresses(20);
   _scrAddrVec1.push_back(TestChain::scrAddrA);
   _scrAddrVec1.push_back(TestChain::scrAddrB);
   _scrAddrVec1.push_back(TestChain::scrAddrC);
   _scrAddrVec1.push_back(TestChain::scrAddrD);
   _scrAddrVec1.push_back(TestChain::scrAddrE);
   _scrAddrVec1.push_back(TestChain::scrAddrF);

   set<BinaryData> scrAddrSet;
   scrAddrSet.insert(_scrAddrVec1.begin(), _scrAddrVec1.end());

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");
      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //mine
      DBTestUtils::mineNewBlock(theBDMt_, TestChain::addrB, 1000);
      pCallback->waitOnSignal(BDMAction_NewBlock);

      //get utxos
      auto utxo_prom = make_shared<promise<vector<UTXO>>>();
      auto utxo_fut = utxo_prom->get_future();
      auto utxo_get = [utxo_prom](ReturnMessage<vector<UTXO>> utxoV)->void
      {
         utxo_prom->set_value(move(utxoV.get()));
      };
      wallet1.getSpendableTxOutListForValue(UINT64_MAX, utxo_get);
      auto&& utxos = utxo_fut.get();

      DBTestUtils::ZcVector zcVec;

      //spend some
      unsigned loopCount = 10;
      unsigned stagger = 0;
      for (auto& utxo : utxos)
      {
         if (utxo.getRecipientScrAddr() != TestChain::scrAddrB ||
            utxo.getScript().getSize() != 25 ||
            utxo.getValue() != 50 * COIN)
            continue;

         //sign
         {
            auto spenderA = make_shared<ScriptSpender>(utxo);
            Signer signer;
            signer.addSpender(spenderA);

            auto id = stagger % _scrAddrVec1.size();

            auto recipient = std::make_shared<Recipient_P2PKH>(
               _scrAddrVec1[id].getSliceCopy(1, 20), utxo.getValue());
            signer.addRecipient(recipient);

            signer.setFeed(feed);
            signer.sign();
            zcVec.push_back(signer.serializeSignedTx(), 130000000, stagger++);
         }

         if (stagger < loopCount)
            continue;

         break;
      }

      DBTestUtils::pushNewZc(theBDMt_, zcVec);
      pCallback->waitOnSignal(BDMAction_ZC);

      auto getAddrOp = [bdvObj, &scrAddrSet](
         unsigned heightOffset, unsigned zcOffset)->OutpointBatch
      {
         auto promPtr = make_shared<promise<OutpointBatch>>();
         auto fut = promPtr->get_future();
         auto addrOpLbd = [promPtr](ReturnMessage<OutpointBatch> batch)->void
         {
            promPtr->set_value(batch.get());
         };

         bdvObj->getOutpointsForAddresses(scrAddrSet, heightOffset, zcOffset, addrOpLbd);
         return fut.get();
      };

      auto computeBalance = [](const vector<OutpointData>& data)->uint64_t
      {
         uint64_t total = 0;
         for (auto& op : data)
         {
            if (op.isSpent_)
               continue;

            total += op.value_;
         }

         return total;
      };

      //check current mined output state
      unsigned heightOffset = 0;
      auto&& addrOp = getAddrOp(heightOffset, UINT32_MAX);
      heightOffset = addrOp.heightCutoff_ + 1;
      ASSERT_EQ(addrOp.outpoints_.size(), 6);

      auto iterAddrA = addrOp.outpoints_.find(TestChain::scrAddrA);
      EXPECT_NE(iterAddrA, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrA->second.size(), 1);
      EXPECT_EQ(computeBalance(iterAddrA->second), 50 * COIN);

      auto iterAddrB = addrOp.outpoints_.find(TestChain::scrAddrB);
      EXPECT_NE(iterAddrB, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrB->second.size(), 1007);
      EXPECT_EQ(computeBalance(iterAddrB->second), 50070 * COIN);

      auto iterAddrC = addrOp.outpoints_.find(TestChain::scrAddrC);
      EXPECT_NE(iterAddrC, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrC->second.size(), 4);
      EXPECT_EQ(computeBalance(iterAddrC->second), 20 * COIN);

      auto iterAddrD = addrOp.outpoints_.find(TestChain::scrAddrD);
      EXPECT_NE(iterAddrD, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrD->second.size(), 4);
      EXPECT_EQ(computeBalance(iterAddrD->second), 65 * COIN);

      auto iterAddrE = addrOp.outpoints_.find(TestChain::scrAddrE);
      EXPECT_NE(iterAddrE, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrE->second.size(), 2);
      EXPECT_EQ(computeBalance(iterAddrE->second), 30 * COIN);

      auto iterAddrF = addrOp.outpoints_.find(TestChain::scrAddrF);
      EXPECT_NE(iterAddrF, addrOp.outpoints_.end());
      EXPECT_EQ(iterAddrF->second.size(), 4);
      EXPECT_EQ(computeBalance(iterAddrF->second), 5 * COIN);

      //check zc outputs
      auto zcAddrOp = getAddrOp(UINT32_MAX, 0);
      ASSERT_EQ(zcAddrOp.outpoints_.size(), loopCount + 1);

      auto iterZcB = zcAddrOp.outpoints_.find(TestChain::scrAddrB);
      ASSERT_NE(iterZcB, zcAddrOp.outpoints_.end());
      EXPECT_EQ(iterZcB->second.size(), 10);

      map<BinaryData, set<unsigned>> zcSpentnessToGet;
      for (unsigned z = 0; z < loopCount; z++)
      {
         auto id = z % _scrAddrVec1.size();
         auto& addr = _scrAddrVec1[id];

         auto addrIter = zcAddrOp.outpoints_.find(addr);
         ASSERT_NE(addrIter, zcAddrOp.outpoints_.end());
         EXPECT_EQ(addrIter->second.size(), 1);

         auto& op = addrIter->second[0];
         EXPECT_EQ(op.value_, 50 * COIN);
         EXPECT_EQ(op.txHeight_, UINT32_MAX);

         auto sptIter = zcSpentnessToGet.find(op.txHash_);
         if (sptIter == zcSpentnessToGet.end())
         {
            sptIter = zcSpentnessToGet.insert(
               make_pair(op.txHash_, set<unsigned>())).first;
         }

         sptIter->second.insert(op.txOutIndex_);
      }

      //grab spentness
      auto getSpentness = [bdvObj](map<BinaryData, set<unsigned>> query)->
         map<BinaryData, map<unsigned, SpentnessResult>>
      {
         auto prom =
            make_shared<promise<map<BinaryData, map<unsigned, SpentnessResult>>>>();
         auto fut = prom->get_future();
         auto lbd = [prom](ReturnMessage<std::map<BinaryData, std::map<unsigned,
            SpentnessResult>>> msg)
         {
            try
            {
               prom->set_value(msg.get());
            }
            catch (ClientMessageError&)
            {
               prom->set_exception(current_exception());
            }
         };

         bdvObj->getSpentnessForOutputs(query, lbd);
         return fut.get();
      };

      map<BinaryData, set<unsigned>> spentnessToGet;
      for (auto& opPair : addrOp.outpoints_)
      {
         for (auto& op : opPair.second)
         {
            auto iter = spentnessToGet.find(op.txHash_);
            if (iter == spentnessToGet.end())
            {
               iter = spentnessToGet.insert(
                  make_pair(op.txHash_, set<unsigned>())).first;
            }

            iter->second.insert(op.txOutIndex_);
         }
      }

      //add an invalid hash
      spentnessToGet.insert(make_pair(BtcUtils::EmptyHash(), set<unsigned>({0})));

      //grab the spentnees
      auto spentnessData = getSpentness(spentnessToGet);

      //check spentness data vs addr outpoint data
      for (auto& opPair : addrOp.outpoints_)
      {
         for (auto& op : opPair.second)
         {
            if (op.isSpent_)
            {
               auto iter = spentnessData.find(op.txHash_);
               ASSERT_NE(iter, spentnessData.end());

               auto idIter = iter->second.find(op.txOutIndex_);
               ASSERT_NE(idIter, iter->second.end());

               EXPECT_EQ(idIter->second.spender_, op.spenderHash_);
               EXPECT_EQ(idIter->second.state_, OutputSpentnessState::Spent);
            }
            else
            {
               auto iter = spentnessData.find(op.txHash_);
               ASSERT_NE(iter, spentnessData.end());

               auto idIter = iter->second.find(op.txOutIndex_);
               ASSERT_NE(idIter, iter->second.end());

               EXPECT_EQ(idIter->second.spender_.getSize(), 0);
               EXPECT_EQ(idIter->second.height_, UINT32_MAX);
               EXPECT_EQ(idIter->second.state_, OutputSpentnessState::Unspent);
            }
         }
      }

      //check the invalid hash
      {
         auto iter = spentnessData.find(BtcUtils::EmptyHash());
         ASSERT_NE(iter, spentnessData.end());

         auto idIter = iter->second.find(0);
         ASSERT_NE(idIter, iter->second.end());

         EXPECT_EQ(idIter->second.spender_.getSize(), 0);
         EXPECT_EQ(idIter->second.height_, UINT32_MAX);
         EXPECT_EQ(idIter->second.state_, OutputSpentnessState::Invalid);
      }

      //sneak in a bad sized hash, should throw
      spentnessToGet.insert(make_pair(READHEX("0011223344"), set<unsigned>()));

      try
      {
         auto spentnessData2 = getSpentness(spentnessToGet);
         ASSERT_FALSE(true);
      }
      catch (ClientMessageError& e)
      {
         EXPECT_EQ(e.what(), string("Error processing command: 84\nmalformed output data"));
      }

      //get zc utxos
      auto zcutxo_prom = make_shared<promise<vector<UTXO>>>();
      auto zcutxo_fut = zcutxo_prom->get_future();
      auto zcutxo_get = [zcutxo_prom](ReturnMessage<vector<UTXO>> utxoV)->void
      {
         zcutxo_prom->set_value(move(utxoV.get()));
      };
      wallet1.getSpendableZCList(zcutxo_get);
      auto&& zcUtxos = zcutxo_fut.get();
      ASSERT_EQ(zcUtxos.size(), loopCount);

      //resolver
      class ResolverUT : public ResolverFeed
      {
      private:
         map<BinaryData, SecureBinaryData> scriptToPub_;
         map<SecureBinaryData, SecureBinaryData> pubToPriv_;

      public:
         ResolverUT(const vector<KeyPair>& keyPairs) :
            ResolverFeed()
         {
            for (auto& keyPair : keyPairs)
            {
               scriptToPub_.insert(make_pair(keyPair.scrHash_, keyPair.pub_));
               pubToPriv_.insert(make_pair(keyPair.pub_, keyPair.priv_));
            }
         }

         BinaryData getByVal(const BinaryData& val) override
         {
            auto iter = scriptToPub_.find(val);
            if (iter == scriptToPub_.end())
               throw std::runtime_error("invalid value");
            return iter->second;
         }

         const SecureBinaryData& getPrivKeyForPubkey(const BinaryData& pubkey) override
         {
            auto iter = pubToPriv_.find(pubkey);
            if (iter == pubToPriv_.end())
               throw std::runtime_error("invalid value");
            return iter->second;
         }
      };

      auto zcFeed = make_shared<ResolverUT>(keyPairs);

      //spend some
      zcVec.clear();
      map<BinaryData, unsigned> newZcHashes;
      unsigned count = 0;
      for (unsigned i=0; i<loopCount; i+=2)
      {
         auto& utxo = zcUtxos[i];

         //sign
         {
            auto spenderA = make_shared<ScriptSpender>(utxo);
            Signer signer;
            signer.addSpender(spenderA);

            auto id = stagger % _scrAddrVec1.size();

            auto recipient = std::make_shared<Recipient_P2PKH>(
               _scrAddrVec1[id].getSliceCopy(1, 20), utxo.getValue());
            signer.addRecipient(recipient);

            signer.setFeed(zcFeed);
            signer.sign();
            auto rawTx = signer.serializeSignedTx();
            Tx tx(rawTx);
            newZcHashes.insert(make_pair(tx.getThisHash(), count++));
            zcVec.push_back(signer.serializeSignedTx(), 130000000, stagger++);
         }
      }

      DBTestUtils::pushNewZc(theBDMt_, zcVec);
      pCallback->waitOnSignal(BDMAction_ZC);  

      //grab new zc output data
      auto newZcAddrOp = getAddrOp(UINT32_MAX, zcAddrOp.zcIndexCutoff_ + 1);
      
      //we create 5 new zc that spend from 5 existing zc and create 5 new 
      //zc outputs, therefor we should have 10 outputs in the batch
      ASSERT_EQ(newZcAddrOp.outpoints_.size(), loopCount);

      //check the output batch
      for (auto& opPair : newZcAddrOp.outpoints_)
      {
         for (auto& op : opPair.second)
         {
            auto iter = newZcHashes.find(op.txHash_);
            if (iter == newZcHashes.end())
            {
               ASSERT_TRUE(op.isSpent_);
               ASSERT_EQ(op.spenderHash_.getSize(), 32);

               auto spenderIter = newZcHashes.find(op.spenderHash_);
               ASSERT_TRUE(spenderIter != newZcHashes.end());
            }
            else
            {
               ASSERT_FALSE(op.isSpent_);
               ASSERT_EQ(op.spenderHash_.getSize(), 0);
            }
         }
      }

      //add invalid hash to the new zc spentness to track
      zcSpentnessToGet.insert(make_pair(BtcUtils::EmptyHash(), set<unsigned>({0})));
      map<BinaryData, map<unsigned, SpentnessResult>> newZcSpentness;
      {
         auto prom =
            make_shared<promise<map<BinaryData, map<unsigned, SpentnessResult>>>>();
         auto fut = prom->get_future();
         auto lbd = [prom](ReturnMessage<map<BinaryData, map<unsigned,
            SpentnessResult>>> msg)
         {
            try
            {
               prom->set_value(msg.get());
            }
            catch (ClientMessageError&)
            {
               prom->set_exception(current_exception());
            }
         };

         bdvObj->getSpentnessForZcOutputs(zcSpentnessToGet, lbd);
         newZcSpentness = move(fut.get());
      }

      //check spentness data vs addr outpoint data
      unsigned spentCount = 0;
      unsigned unspentCount = 0;
      unsigned invalidCount = 0;
      for (auto& zcSpentness : zcSpentnessToGet)
      {
         auto iter = newZcSpentness.find(zcSpentness.first);
         ASSERT_NE(iter, newZcSpentness.end());

         for (auto& zcOp : zcSpentness.second)
         {
            auto idIter = iter->second.find(zcOp);
            ASSERT_NE(idIter, iter->second.end());

            switch (idIter->second.state_)
            {
               case OutputSpentnessState::Spent:
               {
                  auto spenderIter = newZcHashes.find(idIter->second.spender_);
                  EXPECT_NE(spenderIter, newZcHashes.end());
                  EXPECT_EQ(idIter->second.height_, spenderIter->second + loopCount);
                  ++spentCount;
                  break;
               }

               case OutputSpentnessState::Unspent:
               {
                  EXPECT_EQ(idIter->second.spender_.getSize(), 0);

                  ++unspentCount;
                  break;
               }

               case OutputSpentnessState::Invalid:
               {
                  EXPECT_EQ(zcSpentness.first, BtcUtils::EmptyHash());

                  ++invalidCount;
                  break;
               }
            }
         }
      }

      EXPECT_EQ(spentCount, loopCount/2);
      EXPECT_EQ(unspentCount, loopCount/2);
      EXPECT_EQ(invalidCount, 1);

      //check the invalid hash
      {
         auto iter = newZcSpentness.find(BtcUtils::EmptyHash());
         ASSERT_NE(iter, newZcSpentness.end());

         auto idIter = iter->second.find(0);
         ASSERT_NE(idIter, iter->second.end());

         EXPECT_EQ(idIter->second.spender_.getSize(), 0);
         EXPECT_EQ(idIter->second.height_, UINT32_MAX);
         EXPECT_EQ(idIter->second.state_, OutputSpentnessState::Invalid);
      }
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BatchZcChain)
{
   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //balance fetching routine
      vector<string> walletIDs = { wallet1.walletID() };
      auto getBalances = [bdvObj, walletIDs](void)->CombinedBalances
      {
         auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
         auto fut = promPtr->get_future();
         auto balLbd = [promPtr](
            ReturnMessage<map<string, CombinedBalances>> combBal)->void
         {
            promPtr->set_value(combBal.get());
         };

         bdvObj->getCombinedBalances(walletIDs, balLbd);
         auto&& balMap = fut.get();

         if (balMap.size() != 1)
            throw runtime_error("unexpected balance map size");

         return balMap.begin()->second;
      };

      //check balances before pushing zc
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 50 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 70 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 30 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //grab utxos for scrAddrB
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      wallet1.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() != TestChain::scrAddrB)
               continue;

            utxosB.push_back(utxo);
         }
      }

      ASSERT_FALSE(utxosB.empty());

      /*create the transactions*/

      //grab utxo from raw tx
      auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         return utxo;
      };

      BinaryData rawTx1, rawTx2;

      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1 = signer.serializeSignedTx();
      }

      {
         auto utxoD = getUtxoFromRawTx(rawTx1, 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx2 = signer.serializeSignedTx();
      }

      BinaryData rawTx3;
      {
         //10 from E, 5 from F, 3 to A, 2 to E, 5 to D, change to C
         auto zcUtxo1 = getUtxoFromRawTx(rawTx2, 0);
         auto zcUtxo2 = getUtxoFromRawTx(rawTx2, 1);

         Signer signer;
         
         auto spender1 = make_shared<ScriptSpender>(zcUtxo1);
         auto spender2 = make_shared<ScriptSpender>(zcUtxo2);
         signer.addSpender(spender1);
         signer.addSpender(spender2);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 3 * COIN);
         signer.addRecipient(recA);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 2 * COIN);
         signer.addRecipient(recE);
         
         auto recD = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recD);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20),
            spender1->getValue() + spender2->getValue() - 
            recA->getValue() - recE->getValue() - recD->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx3 = signer.serializeSignedTx();
      }

      //batch push tx
      auto broadcastId1 = bdvObj->broadcastZC({ rawTx1, rawTx2, rawTx3 });

      Tx tx1(rawTx1);
      Tx tx2(rawTx2);
      Tx tx3(rawTx3);
      
      set<BinaryData> txHashes;
      txHashes.insert(tx1.getThisHash());
      txHashes.insert(tx2.getThisHash());
      txHashes.insert(tx3.getThisHash());

      set<BinaryData> scrAddrSet;
      scrAddrSet.insert(TestChain::scrAddrA);
      scrAddrSet.insert(TestChain::scrAddrB);
      scrAddrSet.insert(TestChain::scrAddrC);
      scrAddrSet.insert(TestChain::scrAddrD);
      scrAddrSet.insert(TestChain::scrAddrE);
      scrAddrSet.insert(TestChain::scrAddrF);

      pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId1);

      //check balances
      combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 58 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 50 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 25 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 70 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 32 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BatchZcChain_AlreadyInMempool)
{
   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //balance fetching routine
      vector<string> walletIDs = { wallet1.walletID() };
      auto getBalances = [bdvObj, walletIDs](void)->CombinedBalances
      {
         auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
         auto fut = promPtr->get_future();
         auto balLbd = [promPtr](
            ReturnMessage<map<string, CombinedBalances>> combBal)->void
         {
            promPtr->set_value(combBal.get());
         };

         bdvObj->getCombinedBalances(walletIDs, balLbd);
         auto&& balMap = fut.get();

         if (balMap.size() != 1)
            throw runtime_error("unexpected balance map size");

         return balMap.begin()->second;
      };

      //check balances before pushing zc
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 50 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 70 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 30 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //grab utxos for scrAddrB
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      wallet1.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() != TestChain::scrAddrB)
               continue;

            utxosB.push_back(utxo);
         }
      }

      ASSERT_FALSE(utxosB.empty());

      /*create the transactions*/

      //grab utxo from raw tx
      auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         return utxo;
      };

      BinaryData rawTx1, rawTx2;

      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1 = signer.serializeSignedTx();
      }

      {
         auto utxoD = getUtxoFromRawTx(rawTx1, 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx2 = signer.serializeSignedTx();
      }

      BinaryData rawTx3;
      {
         //10 from E, 5 from F, 3 to A, 2 to E, 5 to D, change to C
         auto zcUtxo1 = getUtxoFromRawTx(rawTx2, 0);
         auto zcUtxo2 = getUtxoFromRawTx(rawTx2, 1);

         Signer signer;
         
         auto spender1 = make_shared<ScriptSpender>(zcUtxo1);
         auto spender2 = make_shared<ScriptSpender>(zcUtxo2);
         signer.addSpender(spender1);
         signer.addSpender(spender2);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 3 * COIN);
         signer.addRecipient(recA);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 2 * COIN);
         signer.addRecipient(recE);
         
         auto recD = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recD);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20),
            spender1->getValue() + spender2->getValue() - 
            recA->getValue() - recE->getValue() - recD->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx3 = signer.serializeSignedTx();
      }
      
      Tx tx1(rawTx1);
      Tx tx2(rawTx2);
      Tx tx3(rawTx3);

      //push first tx
      auto broadcastId1 = bdvObj->broadcastZC(rawTx1);

      set<BinaryData> txHashes;
      txHashes.insert(tx1.getThisHash());

      set<BinaryData> scrAddrSet;
      scrAddrSet.insert(TestChain::scrAddrA);
      scrAddrSet.insert(TestChain::scrAddrB);
      scrAddrSet.insert(TestChain::scrAddrD);

      pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId1);

      //batch push all tx
      auto broadcastId2 = bdvObj->broadcastZC({ rawTx1, rawTx2, rawTx3 });
      
      txHashes.clear();
      txHashes.insert(tx2.getThisHash());
      txHashes.insert(tx3.getThisHash());

      scrAddrSet.clear();
      scrAddrSet.insert(TestChain::scrAddrA);
      scrAddrSet.insert(TestChain::scrAddrC);
      scrAddrSet.insert(TestChain::scrAddrD);
      scrAddrSet.insert(TestChain::scrAddrE);
      scrAddrSet.insert(TestChain::scrAddrF);

      //wait on already in mempool error
      pCallback->waitOnError(tx1.getThisHash(), 
         ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool, broadcastId2);

      //wait on zc notifs
      pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId2);

      //check balances
      combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 58 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 50 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 25 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 70 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 32 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BatchZcChain_AlreadyInNodeMempool)
{
   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //balance fetching routine
      vector<string> walletIDs = { wallet1.walletID() };
      auto getBalances = [bdvObj, walletIDs](void)->CombinedBalances
      {
         auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
         auto fut = promPtr->get_future();
         auto balLbd = [promPtr](
            ReturnMessage<map<string, CombinedBalances>> combBal)->void
         {
            promPtr->set_value(combBal.get());
         };

         bdvObj->getCombinedBalances(walletIDs, balLbd);
         auto&& balMap = fut.get();

         if (balMap.size() != 1)
            throw runtime_error("unexpected balance map size");

         return balMap.begin()->second;
      };

      //check balances before pushing zc
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 50 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 70 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 30 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //grab utxos for scrAddrB & scrAddrC
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      wallet1.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB, utxosC;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() == TestChain::scrAddrB)
               utxosB.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrC)
               utxosC.push_back(utxo);
         }
      }

      ASSERT_FALSE(utxosB.empty());
      ASSERT_FALSE(utxosC.empty());

      /*create the transactions*/

      //grab utxo from raw tx
      auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         return utxo;
      };

      BinaryData rawTx1_B;
      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_B = signer.serializeSignedTx();
      }

      BinaryData rawTx1_C;
      {
         //20 from C, 5 to E, change to C
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosC[0]);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_C = signer.serializeSignedTx();
      }

      BinaryData rawTx2;
      {
         auto utxoD = getUtxoFromRawTx(rawTx1_B, 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx2 = signer.serializeSignedTx();
      }

      BinaryData rawTx3;
      {
         //10 from E, 5 from F, 3 to A, 2 to E, 5 to D, change to C
         auto zcUtxo1 = getUtxoFromRawTx(rawTx2, 0);
         auto zcUtxo2 = getUtxoFromRawTx(rawTx2, 1);

         Signer signer;
         
         auto spender1 = make_shared<ScriptSpender>(zcUtxo1);
         auto spender2 = make_shared<ScriptSpender>(zcUtxo2);
         signer.addSpender(spender1);
         signer.addSpender(spender2);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 3 * COIN);
         signer.addRecipient(recA);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 2 * COIN);
         signer.addRecipient(recE);
         
         auto recD = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recD);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20),
            spender1->getValue() + spender2->getValue() - 
            recA->getValue() - recE->getValue() - recD->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx3 = signer.serializeSignedTx();
      }
      
      Tx tx1_B(rawTx1_B);
      Tx tx1_C(rawTx1_C);
      Tx tx2(rawTx2);
      Tx tx3(rawTx3);

      //push through the node
      DBTestUtils::ZcVector zcVec;
      zcVec.push_back(rawTx1_B, 1000000000);
      DBTestUtils::pushNewZc(theBDMt_, zcVec, true);

      //batch push all tx
      auto broadcastId1 = bdvObj->broadcastZC({ rawTx1_B, rawTx1_C, rawTx2, rawTx3 });
      
      set<BinaryData> txHashes;
      txHashes.insert(tx1_B.getThisHash());
      txHashes.insert(tx1_C.getThisHash());
      txHashes.insert(tx2.getThisHash());
      txHashes.insert(tx3.getThisHash());

      set<BinaryData> scrAddrSet;
      scrAddrSet.insert(TestChain::scrAddrA);
      scrAddrSet.insert(TestChain::scrAddrB);
      scrAddrSet.insert(TestChain::scrAddrC);
      scrAddrSet.insert(TestChain::scrAddrD);
      scrAddrSet.insert(TestChain::scrAddrE);
      scrAddrSet.insert(TestChain::scrAddrF);

      //wait on zc notifs
      pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId1);

      //check balances
      combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 58 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 50 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 70 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 37 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BatchZcChain_AlreadyInChain)
{
   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //balance fetching routine
      vector<string> walletIDs = { wallet1.walletID() };
      auto getBalances = [bdvObj, walletIDs](void)->CombinedBalances
      {
         auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
         auto fut = promPtr->get_future();
         auto balLbd = [promPtr](
            ReturnMessage<map<string, CombinedBalances>> combBal)->void
         {
            promPtr->set_value(combBal.get());
         };

         bdvObj->getCombinedBalances(walletIDs, balLbd);
         auto&& balMap = fut.get();

         if (balMap.size() != 1)
            throw runtime_error("unexpected balance map size");

         return balMap.begin()->second;
      };

      //check balances before pushing zc
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 50 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 70 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 30 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //grab utxos for scrAddrB & scrAddrC
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      wallet1.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB, utxosC;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() == TestChain::scrAddrB)
               utxosB.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrC)
               utxosC.push_back(utxo);
         }
      }

      ASSERT_FALSE(utxosB.empty());
      ASSERT_FALSE(utxosC.empty());

      /*create the transactions*/

      //grab utxo from raw tx
      auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         return utxo;
      };

      BinaryData rawTx1_B;
      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_B = signer.serializeSignedTx();
      }

      BinaryData rawTx1_C;
      {
         //20 from C, 5 to E, change to C
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosC[0]);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_C = signer.serializeSignedTx();
      }

      BinaryData rawTx2;
      {
         auto utxoD = getUtxoFromRawTx(rawTx1_B, 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx2 = signer.serializeSignedTx();
      }

      BinaryData rawTx3;
      {
         //10 from E, 5 from F, 3 to A, 2 to E, 5 to D, change to C
         auto zcUtxo1 = getUtxoFromRawTx(rawTx2, 0);
         auto zcUtxo2 = getUtxoFromRawTx(rawTx2, 1);

         Signer signer;
         
         auto spender1 = make_shared<ScriptSpender>(zcUtxo1);
         auto spender2 = make_shared<ScriptSpender>(zcUtxo2);
         signer.addSpender(spender1);
         signer.addSpender(spender2);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 3 * COIN);
         signer.addRecipient(recA);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 2 * COIN);
         signer.addRecipient(recE);
         
         auto recD = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recD);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20),
            spender1->getValue() + spender2->getValue() - 
            recA->getValue() - recE->getValue() - recD->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx3 = signer.serializeSignedTx();
      }
      
      Tx tx1_B(rawTx1_B);
      Tx tx1_C(rawTx1_C);
      Tx tx2(rawTx2);
      Tx tx3(rawTx3);

      //push through the node
      DBTestUtils::ZcVector zcVec;
      zcVec.push_back(rawTx1_B, 1000000000);
      DBTestUtils::pushNewZc(theBDMt_, zcVec, true);

      //mine 1 block
      DBTestUtils::mineNewBlock(theBDMt_, CryptoPRNG::generateRandom(20), 1);
      pCallback->waitOnSignal(BDMAction_NewBlock);

      //batch push all tx
      auto broadcastId1 = bdvObj->broadcastZC({ rawTx1_B, rawTx1_C, rawTx2, rawTx3 });
      
      set<BinaryData> txHashes;
      txHashes.insert(tx1_C.getThisHash());
      txHashes.insert(tx2.getThisHash());
      txHashes.insert(tx3.getThisHash());

      set<BinaryData> scrAddrSet;
      scrAddrSet.insert(TestChain::scrAddrA);
      scrAddrSet.insert(TestChain::scrAddrC);
      scrAddrSet.insert(TestChain::scrAddrD);
      scrAddrSet.insert(TestChain::scrAddrE);
      scrAddrSet.insert(TestChain::scrAddrF);

      //wait on zc notifs
      pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId1);

      //check balances
      combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 58 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 50 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 70 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 37 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BatchZcChain_MissInv)
{
   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //balance fetching routine
      vector<string> walletIDs = { wallet1.walletID() };
      auto getBalances = [bdvObj, walletIDs](void)->CombinedBalances
      {
         auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
         auto fut = promPtr->get_future();
         auto balLbd = [promPtr](
            ReturnMessage<map<string, CombinedBalances>> combBal)->void
         {
            promPtr->set_value(combBal.get());
         };

         bdvObj->getCombinedBalances(walletIDs, balLbd);
         auto&& balMap = fut.get();

         if (balMap.size() != 1)
            throw runtime_error("unexpected balance map size");

         return balMap.begin()->second;
      };

      //check balances before pushing zc
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 50 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 70 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 30 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //grab utxos for scrAddrB & scrAddrC
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      wallet1.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB, utxosC;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() == TestChain::scrAddrB)
               utxosB.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrC)
               utxosC.push_back(utxo);
         }
      }

      ASSERT_FALSE(utxosB.empty());
      ASSERT_FALSE(utxosC.empty());

      /*create the transactions*/

      //grab utxo from raw tx
      auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         return utxo;
      };

      BinaryData rawTx1_B;
      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_B = signer.serializeSignedTx();
      }

      BinaryData rawTx1_C;
      {
         //20 from C, 5 to E, change to C
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosC[0]);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_C = signer.serializeSignedTx();
      }

      BinaryData rawTx2;
      {
         auto utxoD = getUtxoFromRawTx(rawTx1_B, 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx2 = signer.serializeSignedTx();
      }

      BinaryData rawTx3;
      {
         //10 from E, 5 from F, 3 to A, 2 to E, 5 to D, change to C
         auto zcUtxo1 = getUtxoFromRawTx(rawTx2, 0);
         auto zcUtxo2 = getUtxoFromRawTx(rawTx2, 1);

         Signer signer;
         
         auto spender1 = make_shared<ScriptSpender>(zcUtxo1);
         auto spender2 = make_shared<ScriptSpender>(zcUtxo2);
         signer.addSpender(spender1);
         signer.addSpender(spender2);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 3 * COIN);
         signer.addRecipient(recA);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 2 * COIN);
         signer.addRecipient(recE);
         
         auto recD = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recD);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20),
            spender1->getValue() + spender2->getValue() - 
            recA->getValue() - recE->getValue() - recD->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx3 = signer.serializeSignedTx();
      }
      
      Tx tx1_B(rawTx1_B);
      Tx tx1_C(rawTx1_C);
      Tx tx2(rawTx2);
      Tx tx3(rawTx3);

      //push through the node
      nodePtr_->presentZcHash(tx2.getThisHash());

      //batch push all tx
      auto broadcastId1 = bdvObj->broadcastZC({ rawTx1_B, rawTx1_C, rawTx2, rawTx3 });
      
      set<BinaryData> txHashes;
      txHashes.insert(tx1_B.getThisHash());
      txHashes.insert(tx1_C.getThisHash());
      txHashes.insert(tx2.getThisHash());
      txHashes.insert(tx3.getThisHash());

      set<BinaryData> scrAddrSet;
      scrAddrSet.insert(TestChain::scrAddrA);
      scrAddrSet.insert(TestChain::scrAddrB);
      scrAddrSet.insert(TestChain::scrAddrC);
      scrAddrSet.insert(TestChain::scrAddrD);
      scrAddrSet.insert(TestChain::scrAddrE);
      scrAddrSet.insert(TestChain::scrAddrF);

      //wait on zc notifs
      pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId1);

      //check balances
      combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 58 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 50 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 70 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 37 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BatchZcChain_ConflictingChildren)
{
   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //balance fetching routine
      vector<string> walletIDs = { wallet1.walletID() };
      auto getBalances = [bdvObj, walletIDs](void)->CombinedBalances
      {
         auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
         auto fut = promPtr->get_future();
         auto balLbd = [promPtr](
            ReturnMessage<map<string, CombinedBalances>> combBal)->void
         {
            promPtr->set_value(combBal.get());
         };

         bdvObj->getCombinedBalances(walletIDs, balLbd);
         auto&& balMap = fut.get();

         if (balMap.size() != 1)
            throw runtime_error("unexpected balance map size");

         return balMap.begin()->second;
      };

      //check balances before pushing zc
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 50 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 70 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 30 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //grab utxos for scrAddrB & scrAddrC
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      wallet1.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB, utxosC;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() == TestChain::scrAddrB)
               utxosB.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrC)
               utxosC.push_back(utxo);
         }
      }

      ASSERT_FALSE(utxosB.empty());
      ASSERT_FALSE(utxosC.empty());

      /*create the transactions*/

      //grab utxo from raw tx
      auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         return utxo;
      };

      BinaryData rawTx1_B;
      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_B = signer.serializeSignedTx();
      }

      BinaryData rawTx1_C;
      {
         //20 from C, 5 to E, change to C
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosC[0]);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_C = signer.serializeSignedTx();
      }

      BinaryData rawTx2;
      {
         auto utxoD = getUtxoFromRawTx(rawTx1_B, 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx2 = signer.serializeSignedTx();
      }

      BinaryData rawTx3;
      {
         auto utxoD = getUtxoFromRawTx(rawTx1_B, 1);

         //15 from D, 10 to E, change to A
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx3 = signer.serializeSignedTx();
      }
      
      Tx tx1_B(rawTx1_B);
      Tx tx1_C(rawTx1_C);
      Tx tx2(rawTx2);
      Tx tx3(rawTx3);

      //batch push all tx
      auto broadcastId1 = bdvObj->broadcastZC({ rawTx1_B, rawTx1_C, rawTx2, rawTx3 });
      
      set<BinaryData> txHashes;
      txHashes.insert(tx1_B.getThisHash());
      txHashes.insert(tx1_C.getThisHash());
      txHashes.insert(tx2.getThisHash());
      txHashes.insert(tx3.getThisHash());

      set<BinaryData> scrAddrSet;
      scrAddrSet.insert(TestChain::scrAddrA);
      scrAddrSet.insert(TestChain::scrAddrB);
      scrAddrSet.insert(TestChain::scrAddrC);
      scrAddrSet.insert(TestChain::scrAddrD);
      scrAddrSet.insert(TestChain::scrAddrE);
      scrAddrSet.insert(TestChain::scrAddrF);

      //wait on zc error for conflicting child
      pCallback->waitOnError(
         tx3.getThisHash(), ArmoryErrorCodes::ZcBroadcast_VerifyRejected, broadcastId1);

      //wait on zc notifs
      pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId1);

      //check balances
      combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 55 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 50 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 15 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 45 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 10 * COIN);
      }
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BatchZcChain_ConflictingChildren_AlreadyInChain1)
{
   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //balance fetching routine
      vector<string> walletIDs = { wallet1.walletID() };
      auto getBalances = [bdvObj, walletIDs](void)->CombinedBalances
      {
         auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
         auto fut = promPtr->get_future();
         auto balLbd = [promPtr](
            ReturnMessage<map<string, CombinedBalances>> combBal)->void
         {
            promPtr->set_value(combBal.get());
         };

         bdvObj->getCombinedBalances(walletIDs, balLbd);
         auto&& balMap = fut.get();

         if (balMap.size() != 1)
            throw runtime_error("unexpected balance map size");

         return balMap.begin()->second;
      };

      //check balances before pushing zc
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 50 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 70 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 30 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //grab utxos for scrAddrB & scrAddrC
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      wallet1.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB, utxosC;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() == TestChain::scrAddrB)
               utxosB.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrC)
               utxosC.push_back(utxo);
         }
      }

      ASSERT_FALSE(utxosB.empty());
      ASSERT_FALSE(utxosC.empty());

      /*create the transactions*/

      //grab utxo from raw tx
      auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         return utxo;
      };

      BinaryData rawTx1_B;
      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_B = signer.serializeSignedTx();
      }

      BinaryData rawTx1_C;
      {
         //20 from C, 5 to E, change to C
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosC[0]);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_C = signer.serializeSignedTx();
      }

      BinaryData rawTx2;
      {
         auto utxoD = getUtxoFromRawTx(rawTx1_B, 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx2 = signer.serializeSignedTx();
      }

      BinaryData rawTx3;
      {
         auto utxoD = getUtxoFromRawTx(rawTx1_B, 1);

         //15 from D, 10 to E, change to A
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx3 = signer.serializeSignedTx();
      }
      
      Tx tx1_B(rawTx1_B);
      Tx tx1_C(rawTx1_C);
      Tx tx2(rawTx2);
      Tx tx3(rawTx3);

      {
         set<BinaryData> txHashes;
         txHashes.insert(tx1_B.getThisHash());

         set<BinaryData> scrAddrSet;
         scrAddrSet.insert(TestChain::scrAddrA);
         scrAddrSet.insert(TestChain::scrAddrB);
         scrAddrSet.insert(TestChain::scrAddrD);

         //push the first zc
         auto broadcastId1 = bdvObj->broadcastZC(rawTx1_B);

         //wait on notification
         pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId1);
      }
         
      //batch push all tx
      auto broadcastId1 = bdvObj->broadcastZC({ rawTx1_B, rawTx1_C, rawTx2, rawTx3 });
      
      set<BinaryData> txHashes;
      txHashes.insert(tx1_C.getThisHash());
      txHashes.insert(tx2.getThisHash());
      txHashes.insert(tx3.getThisHash());

      set<BinaryData> scrAddrSet;
      scrAddrSet.insert(TestChain::scrAddrC);
      scrAddrSet.insert(TestChain::scrAddrD);
      scrAddrSet.insert(TestChain::scrAddrE);
      scrAddrSet.insert(TestChain::scrAddrF);

      //wait on zc error for conflicting child
      pCallback->waitOnError(
         tx3.getThisHash(), ArmoryErrorCodes::ZcBroadcast_VerifyRejected, broadcastId1);

      //wait on zc notifs
      pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId1);

      //check balances
      combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 55 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 50 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 15 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 45 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 10 * COIN);
      }
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BatchZcChain_ConflictingChildren_AlreadyInChain2)
{
   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //balance fetching routine
      vector<string> walletIDs = { wallet1.walletID() };
      auto getBalances = [bdvObj, walletIDs](void)->CombinedBalances
      {
         auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
         auto fut = promPtr->get_future();
         auto balLbd = [promPtr](
            ReturnMessage<map<string, CombinedBalances>> combBal)->void
         {
            promPtr->set_value(combBal.get());
         };

         bdvObj->getCombinedBalances(walletIDs, balLbd);
         auto&& balMap = fut.get();

         if (balMap.size() != 1)
            throw runtime_error("unexpected balance map size");

         return balMap.begin()->second;
      };

      //check balances before pushing zc
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 50 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 70 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 30 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //grab utxos for scrAddrB & scrAddrC
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      wallet1.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB, utxosC;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() == TestChain::scrAddrB)
               utxosB.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrC)
               utxosC.push_back(utxo);
         }
      }

      ASSERT_FALSE(utxosB.empty());
      ASSERT_FALSE(utxosC.empty());

      /*create the transactions*/

      //grab utxo from raw tx
      auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         return utxo;
      };

      BinaryData rawTx1_B;
      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_B = signer.serializeSignedTx();
      }

      BinaryData rawTx1_C;
      {
         //20 from C, 5 to E, change to C
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosC[0]);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_C = signer.serializeSignedTx();
      }

      BinaryData rawTx2;
      {
         auto utxoD = getUtxoFromRawTx(rawTx1_B, 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx2 = signer.serializeSignedTx();
      }

      BinaryData rawTx3;
      {
         auto utxoD = getUtxoFromRawTx(rawTx1_B, 1);

         //15 from D, 10 to E, change to A
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx3 = signer.serializeSignedTx();
      }
      
      Tx tx1_B(rawTx1_B);
      Tx tx1_C(rawTx1_C);
      Tx tx2(rawTx2);
      Tx tx3(rawTx3);

      {
         set<BinaryData> txHashes;
         txHashes.insert(tx1_B.getThisHash());
         txHashes.insert(tx2.getThisHash());

         set<BinaryData> scrAddrSet;
         scrAddrSet.insert(TestChain::scrAddrA);
         scrAddrSet.insert(TestChain::scrAddrB);
         scrAddrSet.insert(TestChain::scrAddrD);
         scrAddrSet.insert(TestChain::scrAddrE);
         scrAddrSet.insert(TestChain::scrAddrF);

         //push the first zc and its child through the node
         nodePtr_->pushZC({ {rawTx1_B, 0}, {rawTx2, 0} }, false);

         //wait on notification
         pCallback->waitOnZc(txHashes, scrAddrSet, "");
      }
         
      //batch push first zc (already in chain), C (unrelated) 
      //and tx3 (child of first, mempool conflict with tx2)
      auto broadcastId1 = bdvObj->broadcastZC({ rawTx1_B, rawTx1_C, rawTx3 });
      
      set<BinaryData> txHashes;
      txHashes.insert(tx1_C.getThisHash());

      set<BinaryData> scrAddrSet;
      scrAddrSet.insert(TestChain::scrAddrC);
      scrAddrSet.insert(TestChain::scrAddrE);

      //wait on zc error for conflicting child
      pCallback->waitOnError(
         tx3.getThisHash(), ArmoryErrorCodes::ZcBroadcast_VerifyRejected, broadcastId1);

      //wait on zc notifs
      pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId1);

      //check balances
      combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 55 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 50 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 15 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 45 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 10 * COIN);
      }
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BatchZcChain_ConflictingChildren_AlreadyInChain3)
{
   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //balance fetching routine
      vector<string> walletIDs = { wallet1.walletID() };
      auto getBalances = [bdvObj, walletIDs](void)->CombinedBalances
      {
         auto promPtr = make_shared<promise<map<string, CombinedBalances>>>();
         auto fut = promPtr->get_future();
         auto balLbd = [promPtr](
            ReturnMessage<map<string, CombinedBalances>> combBal)->void
         {
            promPtr->set_value(combBal.get());
         };

         bdvObj->getCombinedBalances(walletIDs, balLbd);
         auto&& balMap = fut.get();

         if (balMap.size() != 1)
            throw runtime_error("unexpected balance map size");

         return balMap.begin()->second;
      };

      //check balances before pushing zc
      auto&& combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 50 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 70 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 20 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 30 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 5 * COIN);
      }
   
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //grab utxos for scrAddrB & scrAddrC
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      wallet1.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB, utxosC;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() == TestChain::scrAddrB)
               utxosB.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrC)
               utxosC.push_back(utxo);
         }
      }

      ASSERT_FALSE(utxosB.empty());
      ASSERT_FALSE(utxosC.empty());

      /*create the transactions*/

      //grab utxo from raw tx
      auto getUtxoFromRawTx = [](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         return utxo;
      };

      BinaryData rawTx1_B;
      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_B = signer.serializeSignedTx();
      }

      BinaryData rawTx1_C;
      {
         //20 from C, 5 to E, change to C
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosC[0]);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx1_C = signer.serializeSignedTx();
      }

      BinaryData rawTx2;
      {
         auto utxoD = getUtxoFromRawTx(rawTx1_B, 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx2 = signer.serializeSignedTx();
      }

      BinaryData rawTx3;
      {
         auto utxoD = getUtxoFromRawTx(rawTx1_B, 1);
         auto utxoE = getUtxoFromRawTx(rawTx1_C, 0);

         //15+5 from D & E, 10 to E, change to A
         Signer signer;

         auto spender1 = make_shared<ScriptSpender>(utxoD);
         auto spender2 = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender1);
         signer.addSpender(spender2);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTx3 = signer.serializeSignedTx();
      }
      
      Tx tx1_B(rawTx1_B);
      Tx tx1_C(rawTx1_C);
      Tx tx2(rawTx2);
      Tx tx3(rawTx3);

      {
         set<BinaryData> txHashes;
         txHashes.insert(tx1_B.getThisHash());
         txHashes.insert(tx2.getThisHash());

         set<BinaryData> scrAddrSet;
         scrAddrSet.insert(TestChain::scrAddrA);
         scrAddrSet.insert(TestChain::scrAddrB);
         scrAddrSet.insert(TestChain::scrAddrD);
         scrAddrSet.insert(TestChain::scrAddrE);
         scrAddrSet.insert(TestChain::scrAddrF);

         //push the first zc and its child
         auto broadcastId1 = bdvObj->broadcastZC({ rawTx1_B, rawTx2 });

         //wait on notification
         pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId1);
      }
         
      //batch push first zc (already in chain), C (unrelated) 
      //and tx3 (child of first & C, mempool conflict with tx2 on utxo from first)
      auto broadcastId1 = bdvObj->broadcastZC({ rawTx1_B, rawTx1_C, rawTx3 });
      
      set<BinaryData> txHashes;
      txHashes.insert(tx1_C.getThisHash());

      set<BinaryData> scrAddrSet;
      scrAddrSet.insert(TestChain::scrAddrC);
      scrAddrSet.insert(TestChain::scrAddrE);

      //wait on zc error for conflicting child
      pCallback->waitOnError(
         tx3.getThisHash(), ArmoryErrorCodes::ZcBroadcast_VerifyRejected, broadcastId1);

      //wait on zc notifs
      pCallback->waitOnZc(txHashes, scrAddrSet, broadcastId1);

      //check balances
      combineBalances = getBalances();
      EXPECT_EQ(combineBalances.addressBalances_.size(), 6);

      {
         auto iterA = combineBalances.addressBalances_.find(TestChain::scrAddrA);
         ASSERT_NE(iterA, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterA->second.size(), 3);
         EXPECT_EQ(iterA->second[0], 55 * COIN);

         auto iterB = combineBalances.addressBalances_.find(TestChain::scrAddrB);
         ASSERT_NE(iterB, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterB->second.size(), 3);
         EXPECT_EQ(iterB->second[0], 50 * COIN);

         auto iterC = combineBalances.addressBalances_.find(TestChain::scrAddrC);
         ASSERT_NE(iterC, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterC->second.size(), 3);
         EXPECT_EQ(iterC->second[0], 15 * COIN);

         auto iterD = combineBalances.addressBalances_.find(TestChain::scrAddrD);
         ASSERT_NE(iterD, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterD->second.size(), 3);
         EXPECT_EQ(iterD->second[0], 65 * COIN);

         auto iterE = combineBalances.addressBalances_.find(TestChain::scrAddrE);
         ASSERT_NE(iterE, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterE->second.size(), 3);
         EXPECT_EQ(iterE->second[0], 45 * COIN);

         auto iterF = combineBalances.addressBalances_.find(TestChain::scrAddrF);
         ASSERT_NE(iterF, combineBalances.addressBalances_.end());
         ASSERT_EQ(iterF->second.size(), 3);
         EXPECT_EQ(iterF->second[0], 10 * COIN);
      }
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BroadcastAlreadyMinedTx)
{
   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      //grab a mined tx with unspent outputs
      auto&& ZC1 = TestUtils::getTx(5, 2); //block 5, tx 2
      auto&& ZChash1 = BtcUtils::getHash256(ZC1);

      //and one with spent outputs
      auto&& ZC2 = TestUtils::getTx(2, 1); //block 5, tx 2
      auto&& ZChash2 = BtcUtils::getHash256(ZC2);

      //try and broadcast both
      auto broadcastId1 = bdvObj->broadcastZC({ZC1, ZC2});

      //wait on zc errors
      pCallback->waitOnError(ZChash1, 
         ArmoryErrorCodes::ZcBroadcast_AlreadyInChain, broadcastId1);
      
      pCallback->waitOnError(ZChash2, 
         ArmoryErrorCodes::ZcBroadcast_AlreadyInChain, broadcastId1);
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BroadcastSameZC_ManyThreads)
{
   struct WSClient
   {
      shared_ptr<AsyncClient::BlockDataViewer> bdvPtr_;
      AsyncClient::BtcWallet wlt_;
      shared_ptr<DBTestUtils::UTCallback> callbackPtr_;

      WSClient(
         shared_ptr<AsyncClient::BlockDataViewer> bdvPtr, 
         AsyncClient::BtcWallet& wlt,
         shared_ptr<DBTestUtils::UTCallback> callbackPtr) :
         bdvPtr_(bdvPtr), wlt_(move(wlt)), callbackPtr_(callbackPtr)
      {}
   };

   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   //create BDV lambda
   auto setupBDV = [this, &serverPubkey](void)->shared_ptr<WSClient>
   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      auto client = make_shared<WSClient>(bdvObj, wallet1, pCallback);
      return client;
   };

   //create main bdv instance
   auto mainInstance = setupBDV();

   /*
   create a batch of zc with chains:
      1-2-3
      1-4 (4 conflicts with 2)
      5-6
      7
   */

   vector<BinaryData> rawTxVec, zcHashes;
   map<BinaryData, map<unsigned, UTXO>> outputMap;
   {
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //utxo from raw tx lambda
      auto getUtxoFromRawTx = [&outputMap](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         auto& idMap = outputMap[utxo.txHash_];
         idMap[id] = utxo;

         return utxo;
      };

      //grab utxos for scrAddrB, scrAddrC, scrAddrE
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      mainInstance->wlt_.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB, utxosC, utxosE;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() == TestChain::scrAddrB)
               utxosB.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrC)
               utxosC.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrE)
               utxosE.push_back(utxo);

            auto& idMap = outputMap[utxo.txHash_];
            idMap[utxo.txOutIndex_] = utxo;
         }
      }

      ASSERT_FALSE(utxosB.empty());
      ASSERT_FALSE(utxosC.empty());
      ASSERT_FALSE(utxosE.empty());

      /*create the transactions*/

      //1
      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }

      //2
      {
         auto utxoD = getUtxoFromRawTx(rawTxVec[0], 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }
      
      //3
      {
         auto utxoF = getUtxoFromRawTx(rawTxVec[1], 1);

         //5 from F, 5 to B
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoF);
         signer.addSpender(spender);

         auto recB = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrB.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recB);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }

      //4
      {
         auto utxoA = getUtxoFromRawTx(rawTxVec[0], 1);

         //15 from D, 14 to C
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoA);
         signer.addSpender(spender);

         auto recC = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20), 14 * COIN);
         signer.addRecipient(recC);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }

      //5
      {
         //10 from C, 10 to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosC[0]);
         signer.addSpender(spender);

         auto recD = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recD);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }

      //6
      {
         auto utxoD = getUtxoFromRawTx(rawTxVec[4], 0);

         //10 from D, 5 to F, change to A
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recF = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recF);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 
            spender->getValue() - recF->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }

      //7
      {
         //20 from E, 10 to F, change to A
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosE[0]);
         signer.addSpender(spender);

         auto recF = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recF);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 
            spender->getValue() - recF->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }      
   }

   //3 case1, 3 case2, 1 case3, 3 case4, 3 case5
   unsigned N = 13;

   //create N side instances
   vector<shared_ptr<WSClient>> sideInstances;
   for (unsigned i=0; i<N; i++)
      sideInstances.emplace_back(setupBDV());

   //get addresses for tx lambda
   auto getAddressesForRawTx = [&outputMap](const Tx& tx)->set<BinaryData>
   {
      set<BinaryData> addrSet;

      for (unsigned i=0; i<tx.getNumTxIn(); i++)
      {
         auto txin = tx.getTxInCopy(i);
         auto op = txin.getOutPoint();

         auto hashIter = outputMap.find(op.getTxHash());
         EXPECT_TRUE(hashIter != outputMap.end());

         auto idIter = hashIter->second.find(op.getTxOutIndex());
         EXPECT_TRUE(idIter != hashIter->second.end());

         auto& utxo = idIter->second;
         addrSet.insert(utxo.getRecipientScrAddr());
      }

      for (unsigned i=0; i<tx.getNumTxOut(); i++)
      {
         auto txout = tx.getTxOutCopy(i);
         addrSet.insert(txout.getScrAddressStr());
      }

      return addrSet;
   };

   set<BinaryData> mainScrAddrSet;
   set<BinaryData> mainHashes;   
   {
      vector<unsigned> zcIds = {1, 2, 3, 5, 6};
      for (auto& id : zcIds)
      {
         Tx tx(rawTxVec[id - 1]);
         mainHashes.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         mainScrAddrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }   
   }

   //case 1
   auto case1 = [&](unsigned instanceId)->void
   {
      auto instance = sideInstances[instanceId];

      //push 1-2-3
      vector<unsigned> zcIds = {1, 2, 3};

      //ids for the zc we are not broadcasting but which addresses we watch
      vector<unsigned> zcIds_skipped = {5, 6};

      vector<BinaryData> zcs;
      set<BinaryData> addrSet;
      set<BinaryData> hashSet;
      for (auto& id : zcIds)
      {
         zcs.push_back(rawTxVec[id - 1]);
         Tx tx(rawTxVec[id - 1]);
         hashSet.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }

      set<BinaryData> addrSet_skipped;
      set<BinaryData> hashSet_skipped;
      for (auto& id : zcIds_skipped)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet_skipped.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet_skipped.insert(localAddrSet.begin(), localAddrSet.end());
      }

      auto broadcastId1 = instance->bdvPtr_->broadcastZC(zcs);

      //wait on broadcast errors
      map<BinaryData, ArmoryErrorCodes> errorMap;
      for (auto& id : zcIds)
         errorMap.emplace(zcHashes[id - 1], ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool);
      instance->callbackPtr_->waitOnErrors(errorMap, broadcastId1);

      //wait on zc
      instance->callbackPtr_->waitOnZc(hashSet_skipped, addrSet_skipped, "");
      instance->callbackPtr_->waitOnZc(hashSet, addrSet, broadcastId1);
   };

   //case 2
   auto case2 = [&](unsigned instanceId)->void
   {
      auto instance = sideInstances[instanceId];

      //push 5-6
      vector<unsigned> zcIds = {5, 6};
      vector<unsigned> zcIds_skipped = {1, 2, 3};

      vector<BinaryData> zcs;
      set<BinaryData> addrSet;
      set<BinaryData> hashSet;
      for (auto& id : zcIds)
      {
         zcs.push_back(rawTxVec[id - 1]);
         Tx tx(rawTxVec[id - 1]);
         hashSet.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }   

      set<BinaryData> addrSet_skipped;
      set<BinaryData> hashSet_skipped;
      for (auto& id : zcIds_skipped)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet_skipped.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet_skipped.insert(localAddrSet.begin(), localAddrSet.end());
      }

      auto broadcastId1 = instance->bdvPtr_->broadcastZC(zcs);

      //wait on broadcast errors
      map<BinaryData, ArmoryErrorCodes> errorMap;
      for (auto& id : zcIds)
         errorMap.emplace(zcHashes[id - 1], ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool);
      instance->callbackPtr_->waitOnErrors(errorMap, broadcastId1);

      //wait on zc
      instance->callbackPtr_->waitOnZc(hashSet_skipped, addrSet_skipped, "");
      instance->callbackPtr_->waitOnZc(hashSet, addrSet, broadcastId1);
   };

   //case 3
   auto case3 = [&](unsigned instanceId)->void
   {
      auto instance = sideInstances[instanceId];

      //push 1-4 7
      auto broadcastId1 = instance->bdvPtr_->broadcastZC({
         rawTxVec[0], rawTxVec[3],
         rawTxVec[6]
      });

      //don't grab 4 as it can't broadcast
      vector<unsigned> zcIds = {1, 7};
      vector<unsigned> zcIds_skipped = {2, 3, 5, 6};

      set<BinaryData> addrSet;
      set<BinaryData> hashSet;
      for (auto& id : zcIds)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }  

      set<BinaryData> addrSet_skipped;
      set<BinaryData> hashSet_skipped;
      for (auto& id : zcIds_skipped)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet_skipped.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet_skipped.insert(localAddrSet.begin(), localAddrSet.end());
      }


      //wait on broadcast errors
      instance->callbackPtr_->waitOnError(
         zcHashes[0], ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool, broadcastId1);

      instance->callbackPtr_->waitOnError(
         zcHashes[3], ArmoryErrorCodes::ZcBroadcast_VerifyRejected, broadcastId1);

      //wait on zc
      instance->callbackPtr_->waitOnZc(hashSet_skipped, addrSet_skipped, "");

      //wait on 7
      instance->callbackPtr_->waitOnZc(hashSet, addrSet, broadcastId1);
   };

   //case 4
   auto case4 = [&](unsigned instanceId)->void
   {
      auto instance = sideInstances[instanceId];

      //push 5-6 7
      vector<unsigned> zcIds = {5, 6, 7};
      vector<unsigned> zcIds_skipped = {1, 2, 3};

      vector<BinaryData> zcs;
      set<BinaryData> addrSet;
      set<BinaryData> hashSet;
      for (auto& id : zcIds)
      {
         zcs.push_back(rawTxVec[id - 1]);
         Tx tx(rawTxVec[id - 1]);
         hashSet.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }

      set<BinaryData> addrSet_skipped;
      set<BinaryData> hashSet_skipped;
      for (auto& id : zcIds_skipped)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet_skipped.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet_skipped.insert(localAddrSet.begin(), localAddrSet.end());
      }

      auto broadcastId1 = instance->bdvPtr_->broadcastZC(zcs);

      //wait on broadcast errors
      map<BinaryData, ArmoryErrorCodes> errorMap;
      for (auto& id : zcIds)
         errorMap.emplace(zcHashes[id - 1], ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool);
      instance->callbackPtr_->waitOnErrors(errorMap, broadcastId1);

      //wait on zc
      instance->callbackPtr_->waitOnZc(hashSet_skipped, addrSet_skipped, "");
      instance->callbackPtr_->waitOnZc(hashSet, addrSet, broadcastId1);
   };

   //case 5
   auto case5 = [&](unsigned instanceId)->void
   {
      auto instance = sideInstances[instanceId];

      //push 4 5-6
      auto broadcastId1 = instance->bdvPtr_->broadcastZC({
         rawTxVec[3], 
         rawTxVec[4], rawTxVec[5]
      });

      //skip 4 as it can't broadcast
      vector<unsigned> zcIds = {5, 6};
      vector<unsigned> zcIds_skipped = {1, 2, 3};

      set<BinaryData> addrSet;
      set<BinaryData> hashSet;
      for (auto& id : zcIds)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }

      set<BinaryData> addrSet_skipped;
      set<BinaryData> hashSet_skipped;
      for (auto& id : zcIds_skipped)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet_skipped.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet_skipped.insert(localAddrSet.begin(), localAddrSet.end());
      }

      //wait on broadcast errors
      map<BinaryData, ArmoryErrorCodes> errorMap;
      for (auto& id : zcIds)
         errorMap.emplace(zcHashes[id - 1], ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool);
      instance->callbackPtr_->waitOnErrors(errorMap, broadcastId1);

      instance->callbackPtr_->waitOnError(
         zcHashes[3], ArmoryErrorCodes::ZcBroadcast_VerifyRejected, broadcastId1);

      //wait on zc
      instance->callbackPtr_->waitOnZc(hashSet_skipped, addrSet_skipped, "");
      instance->callbackPtr_->waitOnZc(hashSet, addrSet, broadcastId1);
   };

   //main instance
   {
      //set zc inv delay, this will allow for batches in side jobs to 
      //collide with the original one
      nodePtr_->stallNextZc(3); //in seconds

      //push 1-2-3 & 5-6
      vector<unsigned> zcIds = {1, 2, 3, 5, 6};

      vector<BinaryData> zcs;
      set<BinaryData> scrAddrSet;
      set<BinaryData> hashes;
      for (auto& id : zcIds)
      {
         zcs.push_back(rawTxVec[id - 1]);
         Tx tx(zcs.back());
         hashes.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         scrAddrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }

      auto broadcastId1 = mainInstance->bdvPtr_->broadcastZC(zcs);
      
      /*
      delay for 1 second before starting side jobs to make sure the 
      primary broadcast is first in line
      */
      this_thread::sleep_for(chrono::seconds(1));

      //start the side jobs
      vector<thread> threads;
      for (unsigned i=0; i<3; i++)
         threads.push_back(thread(case1, i));

      for (unsigned i=3; i<6; i++)
         threads.push_back(thread(case2, i));

      //needs case3 to broadcast before case 4
      threads.push_back(thread(case3, 6));
      this_thread::sleep_for(chrono::milliseconds(500));

      for (unsigned i=7; i<10; i++)
         threads.push_back(thread(case4, i));

      for (unsigned i=10; i<13; i++)
         threads.push_back(thread(case5, i));

      //wait on zc
      mainInstance->callbackPtr_->waitOnZc(hashes, scrAddrSet, broadcastId1);

      //wait on side jobs
      for (auto& thr : threads)
      {
         if (thr.joinable())
            thr.join();
      }

      //done
   }


   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BroadcastSameZC_ManyThreads_RPCFallback)
{
   struct WSClient
   {
      shared_ptr<AsyncClient::BlockDataViewer> bdvPtr_;
      AsyncClient::BtcWallet wlt_;
      shared_ptr<DBTestUtils::UTCallback> callbackPtr_;

      WSClient(
         shared_ptr<AsyncClient::BlockDataViewer> bdvPtr, 
         AsyncClient::BtcWallet& wlt,
         shared_ptr<DBTestUtils::UTCallback> callbackPtr) :
         bdvPtr_(bdvPtr), wlt_(move(wlt)), callbackPtr_(callbackPtr)
      {}
   };

   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   //create BDV lambda
   auto setupBDV = [this, &serverPubkey](void)->shared_ptr<WSClient>
   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      auto client = make_shared<WSClient>(bdvObj, wallet1, pCallback);
      return client;
   };

   //create main bdv instance
   auto mainInstance = setupBDV();

   /*
   create a batch of zc with chains:
      1-2-3
      1-4 (4 conflicts with 2)
      5-6
      7
   */

   vector<BinaryData> rawTxVec, zcHashes;
   map<BinaryData, map<unsigned, UTXO>> outputMap;
   {
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //utxo from raw tx lambda
      auto getUtxoFromRawTx = [&outputMap](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         auto& idMap = outputMap[utxo.txHash_];
         idMap[id] = utxo;

         return utxo;
      };

      //grab utxos for scrAddrB, scrAddrC, scrAddrE
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      mainInstance->wlt_.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB, utxosC, utxosE;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() == TestChain::scrAddrB)
               utxosB.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrC)
               utxosC.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrE)
               utxosE.push_back(utxo);

            auto& idMap = outputMap[utxo.txHash_];
            idMap[utxo.txOutIndex_] = utxo;
         }
      }

      ASSERT_FALSE(utxosB.empty());
      ASSERT_FALSE(utxosC.empty());
      ASSERT_FALSE(utxosE.empty());

      /*create the transactions*/

      //1
      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }

      //2
      {
         auto utxoD = getUtxoFromRawTx(rawTxVec[0], 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }
      
      //3
      {
         auto utxoF = getUtxoFromRawTx(rawTxVec[1], 1);

         //5 from F, 5 to B
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoF);
         signer.addSpender(spender);

         auto recB = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrB.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recB);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }

      //4
      {
         auto utxoA = getUtxoFromRawTx(rawTxVec[0], 1);

         //15 from D, 14 to C
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoA);
         signer.addSpender(spender);

         auto recC = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrC.getSliceCopy(1, 20), 14 * COIN);
         signer.addRecipient(recC);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }

      //5
      {
         //10 from C, 10 to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosC[0]);
         signer.addSpender(spender);

         auto recD = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recD);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }

      //6
      {
         auto utxoD = getUtxoFromRawTx(rawTxVec[4], 0);

         //10 from D, 5 to F, change to A
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recF = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recF);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 
            spender->getValue() - recF->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }

      //7
      {
         //20 from E, 10 to F, change to A
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosE[0]);
         signer.addSpender(spender);

         auto recF = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recF);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 
            spender->getValue() - recF->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }
   }

   //3 case1, 3 case2, 1 case3, 3 case4, 3 case5
   unsigned N = 13;

   //create N side instances
   vector<shared_ptr<WSClient>> sideInstances;
   for (unsigned i=0; i<N; i++)
      sideInstances.emplace_back(setupBDV());

   //get addresses for tx lambda
   auto getAddressesForRawTx = [&outputMap](const Tx& tx)->set<BinaryData>
   {
      set<BinaryData> addrSet;

      for (unsigned i=0; i<tx.getNumTxIn(); i++)
      {
         auto txin = tx.getTxInCopy(i);
         auto op = txin.getOutPoint();

         auto hashIter = outputMap.find(op.getTxHash());
         EXPECT_TRUE(hashIter != outputMap.end());

         auto idIter = hashIter->second.find(op.getTxOutIndex());
         EXPECT_TRUE(idIter != hashIter->second.end());

         auto& utxo = idIter->second;
         addrSet.insert(utxo.getRecipientScrAddr());
      }

      for (unsigned i=0; i<tx.getNumTxOut(); i++)
      {
         auto txout = tx.getTxOutCopy(i);
         addrSet.insert(txout.getScrAddressStr());
      }

      return addrSet;
   };

   set<BinaryData> mainScrAddrSet;
   set<BinaryData> mainHashes;   
   {
      vector<unsigned> zcIds = {1, 2, 3, 5, 6};
      for (auto& id : zcIds)
      {
         Tx tx(rawTxVec[id - 1]);
         mainHashes.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         mainScrAddrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }   
   }

   //case 1
   auto case1 = [&](unsigned instanceId)->void
   {
      auto instance = sideInstances[instanceId];

      //push 1-2-3
      vector<unsigned> zcIds = {1, 2, 3};

      //ids for the zc we are not broadcasting but which addresses we watch
      vector<unsigned> zcIds_skipped = {5, 6};

      vector<BinaryData> zcs;
      set<BinaryData> addrSet;
      set<BinaryData> hashSet;
      for (auto& id : zcIds)
      {
         zcs.push_back(rawTxVec[id - 1]);
         Tx tx(rawTxVec[id - 1]);
         hashSet.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }

      set<BinaryData> addrSet_skipped;
      set<BinaryData> hashSet_skipped;
      for (auto& id : zcIds_skipped)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet_skipped.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet_skipped.insert(localAddrSet.begin(), localAddrSet.end());
      }

      auto broadcastId1 = instance->bdvPtr_->broadcastZC(zcs);

      //wait on zc
      instance->callbackPtr_->waitOnZc(hashSet, addrSet, broadcastId1);
      instance->callbackPtr_->waitOnZc(hashSet_skipped, addrSet_skipped, "");

      //wait on broadcast errors
      map<BinaryData, ArmoryErrorCodes> errorMap;
      for (auto& id : zcIds)
         errorMap.emplace(zcHashes[id - 1], ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool);
      instance->callbackPtr_->waitOnErrors(errorMap, broadcastId1);
   };

   //case 2
   auto case2 = [&](unsigned instanceId)->void
   {
      auto instance = sideInstances[instanceId];

      //push 5-6
      vector<unsigned> zcIds = {5, 6};
      vector<unsigned> zcIds_skipped = {1, 2, 3};

      vector<BinaryData> zcs;
      set<BinaryData> addrSet;
      set<BinaryData> hashSet;
      for (auto& id : zcIds)
      {
         zcs.push_back(rawTxVec[id - 1]);
         Tx tx(rawTxVec[id - 1]);
         hashSet.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }   

      set<BinaryData> addrSet_skipped;
      set<BinaryData> hashSet_skipped;
      for (auto& id : zcIds_skipped)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet_skipped.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet_skipped.insert(localAddrSet.begin(), localAddrSet.end());
      }

      auto broadcastId1 = instance->bdvPtr_->broadcastZC(zcs);

      //wait on zc
      instance->callbackPtr_->waitOnZc(hashSet_skipped, addrSet_skipped, "");
      instance->callbackPtr_->waitOnZc(hashSet, addrSet, broadcastId1);

      //wait on broadcast errors
      map<BinaryData, ArmoryErrorCodes> errorMap;
      for (auto& id : zcIds)
         errorMap.emplace(zcHashes[id - 1], ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool);
      instance->callbackPtr_->waitOnErrors(errorMap, broadcastId1);

   };

   //case 3
   auto case3 = [&](unsigned instanceId)->void
   {
      auto instance = sideInstances[instanceId];

      //push 1-4 7
      auto broadcastId1 = instance->bdvPtr_->broadcastZC({
         rawTxVec[0], rawTxVec[3],
         rawTxVec[6]
      });

      //don't grab 4 as it can't broadcast
      vector<unsigned> zcIds = {1, 7};
      vector<unsigned> zcIds_skipped = {2, 3, 5, 6};

      set<BinaryData> addrSet;
      set<BinaryData> hashSet;
      for (auto& id : zcIds)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }  

      set<BinaryData> addrSet_skipped;
      set<BinaryData> hashSet_skipped;
      for (auto& id : zcIds_skipped)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet_skipped.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet_skipped.insert(localAddrSet.begin(), localAddrSet.end());
      }

      //wait on zc
      instance->callbackPtr_->waitOnZc_OutOfOrder(hashSet_skipped, "");

      //wait on 7
      instance->callbackPtr_->waitOnZc_OutOfOrder(hashSet, broadcastId1);

      //wait on broadcast errors
      instance->callbackPtr_->waitOnError(
         zcHashes[0], ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool, broadcastId1);

      instance->callbackPtr_->waitOnError(
         zcHashes[3], ArmoryErrorCodes::ZcBroadcast_VerifyRejected, broadcastId1);
   };

   //case 4
   auto case4 = [&](unsigned instanceId)->void
   {
      auto instance = sideInstances[instanceId];

      //push 5-6 7
      vector<unsigned> zcIds = {5, 6, 7};
      vector<unsigned> zcIds_skipped = {1, 2, 3};

      vector<BinaryData> zcs;
      set<BinaryData> addrSet;
      set<BinaryData> hashSet;
      for (auto& id : zcIds)
      {
         zcs.push_back(rawTxVec[id - 1]);
         Tx tx(rawTxVec[id - 1]);
         hashSet.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }

      set<BinaryData> addrSet_skipped;
      set<BinaryData> hashSet_skipped;
      for (auto& id : zcIds_skipped)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet_skipped.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet_skipped.insert(localAddrSet.begin(), localAddrSet.end());
      }

      auto broadcastId1 = instance->bdvPtr_->broadcastZC(zcs);

      //wait on broadcast errors
      map<BinaryData, ArmoryErrorCodes> errorMap;
      for (auto& id : zcIds)
         errorMap.emplace(zcHashes[id - 1], ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool);
      instance->callbackPtr_->waitOnErrors(errorMap, broadcastId1);

      //wait on zc
      instance->callbackPtr_->waitOnZc(hashSet_skipped, addrSet_skipped, "");
      instance->callbackPtr_->waitOnZc(hashSet, addrSet, broadcastId1);
   };

   //case 5
   auto case5 = [&](unsigned instanceId)->void
   {
      auto instance = sideInstances[instanceId];

      //push 4 5-6
      auto broadcastId1 = instance->bdvPtr_->broadcastZC({
         rawTxVec[3], 
         rawTxVec[4], rawTxVec[5]
      });

      //skip 4 as it can't broadcast
      vector<unsigned> zcIds = {5, 6};
      vector<unsigned> zcIds_skipped = {1, 2, 3};

      set<BinaryData> addrSet;
      set<BinaryData> hashSet;
      for (auto& id : zcIds)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }

      set<BinaryData> addrSet_skipped;
      set<BinaryData> hashSet_skipped;
      for (auto& id : zcIds_skipped)
      {
         Tx tx(rawTxVec[id - 1]);
         hashSet_skipped.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet_skipped.insert(localAddrSet.begin(), localAddrSet.end());
      }

      //wait on broadcast errors
      map<BinaryData, ArmoryErrorCodes> errorMap;
      for (auto& id : zcIds)
         errorMap.emplace(zcHashes[id - 1], ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool);
      instance->callbackPtr_->waitOnErrors(errorMap, broadcastId1);

      instance->callbackPtr_->waitOnError(
         zcHashes[3], ArmoryErrorCodes::ZcBroadcast_VerifyRejected, broadcastId1);

      //wait on zc
      instance->callbackPtr_->waitOnZc(hashSet_skipped, addrSet_skipped, "");
      instance->callbackPtr_->waitOnZc(hashSet, addrSet, broadcastId1);
   };

   //main instance
   {
      //skip all zc to force a RPC fallback
      nodePtr_->skipZc(100000);

      //push 1-2-3 & 5-6
      vector<unsigned> zcIds = {1, 2, 3, 5, 6};

      vector<BinaryData> zcs;
      set<BinaryData> scrAddrSet;
      set<BinaryData> hashes;
      for (auto& id : zcIds)
      {
         zcs.push_back(rawTxVec[id - 1]);
         Tx tx(zcs.back());
         hashes.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         scrAddrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }

      auto broadcastId1 = mainInstance->bdvPtr_->broadcastZC(zcs);
      
      /*
      delay for 1 second before starting side jobs to make sure the 
      primary broadcast is first in line
      */
      this_thread::sleep_for(chrono::seconds(1));

      //start the side jobs
      vector<thread> threads;
      for (unsigned i=0; i<3; i++)
         threads.push_back(thread(case1, i));

      for (unsigned i=3; i<6; i++)
         threads.push_back(thread(case2, i));

      //needs case3 to broadcast before case 4
      threads.push_back(thread(case3, 6));
      this_thread::sleep_for(chrono::milliseconds(500));

      for (unsigned i=7; i<10; i++)
         threads.push_back(thread(case4, i));

      for (unsigned i=10; i<13; i++)
         threads.push_back(thread(case5, i));

      //wait on zc
      mainInstance->callbackPtr_->waitOnZc(hashes, scrAddrSet, broadcastId1);

      //wait on side jobs
      for (auto& thr : threads)
      {
         if (thr.joinable())
            thr.join();
      }

      //done
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
TEST_F(WebSocketTests, WebSocketStack_BroadcastSameZC_RPCThenP2P)
{
   struct WSClient
   {
      shared_ptr<AsyncClient::BlockDataViewer> bdvPtr_;
      AsyncClient::BtcWallet wlt_;
      shared_ptr<DBTestUtils::UTCallback> callbackPtr_;

      WSClient(
         shared_ptr<AsyncClient::BlockDataViewer> bdvPtr, 
         AsyncClient::BtcWallet& wlt,
         shared_ptr<DBTestUtils::UTCallback> callbackPtr) :
         bdvPtr_(bdvPtr), wlt_(move(wlt)), callbackPtr_(callbackPtr)
      {}
   };

   //public server
   startupBIP150CTX(4, true);

   TestUtils::setBlocks({ "0", "1", "2", "3", "4", "5" }, blk0dat_);
   WebSocketServer::initAuthPeers(authPeersPassLbd_);
   WebSocketServer::start(theBDMt_, true);
   auto&& serverPubkey = WebSocketServer::getPublicKey();
   theBDMt_->start(config.initMode_);

   //create BDV lambda
   auto setupBDV = [this, &serverPubkey](void)->shared_ptr<WSClient>
   {
      auto pCallback = make_shared<DBTestUtils::UTCallback>();
      auto&& bdvObj = AsyncClient::BlockDataViewer::getNewBDV(
         "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
         authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, pCallback);
      bdvObj->addPublicKey(serverPubkey);
      bdvObj->connectToRemote();
      bdvObj->registerWithDB(NetworkConfig::getMagicBytes());

      auto&& wallet1 = bdvObj->instantiateWallet("wallet1");

      vector<BinaryData> _scrAddrVec1;
      _scrAddrVec1.push_back(TestChain::scrAddrA);
      _scrAddrVec1.push_back(TestChain::scrAddrB);
      _scrAddrVec1.push_back(TestChain::scrAddrC);
      _scrAddrVec1.push_back(TestChain::scrAddrD);
      _scrAddrVec1.push_back(TestChain::scrAddrE);
      _scrAddrVec1.push_back(TestChain::scrAddrF);

      vector<string> walletRegIDs;
      walletRegIDs.push_back(
         wallet1.registerAddresses(_scrAddrVec1, false));

      //wait on registration ack
      pCallback->waitOnManySignals(BDMAction_Refresh, walletRegIDs);

      //go online
      bdvObj->goOnline();
      pCallback->waitOnSignal(BDMAction_Ready);

      auto client = make_shared<WSClient>(bdvObj, wallet1, pCallback);
      return client;
   };

   //create main bdv instance
   auto mainInstance = setupBDV();

   /*
   create a batch of zc with chains:
      1-2
      3
   */

   vector<BinaryData> rawTxVec, zcHashes;
   map<BinaryData, map<unsigned, UTXO>> outputMap;
   {
      //instantiate resolver feed overloaded object
      auto feed = make_shared<ResolverUtils::TestResolverFeed>();

      auto addToFeed = [feed](const BinaryData& key)->void
      {
         auto&& datapair = DBTestUtils::getAddrAndPubKeyFromPrivKey(key);
         feed->h160ToPubKey_.insert(datapair);
         feed->pubKeyToPrivKey_[datapair.second] = key;
      };

      addToFeed(TestChain::privKeyAddrB);
      addToFeed(TestChain::privKeyAddrC);
      addToFeed(TestChain::privKeyAddrD);
      addToFeed(TestChain::privKeyAddrE);
      addToFeed(TestChain::privKeyAddrF);

      //utxo from raw tx lambda
      auto getUtxoFromRawTx = [&outputMap](BinaryData& rawTx, unsigned id)->UTXO
      {
         Tx tx(rawTx);
         if (id > tx.getNumTxOut())
            throw runtime_error("invalid txout count");

         auto&& txOut = tx.getTxOutCopy(id);

         UTXO utxo;
         utxo.unserializeRaw(txOut.serialize());
         utxo.txOutIndex_ = id;
         utxo.txHash_ = tx.getThisHash();

         auto& idMap = outputMap[utxo.txHash_];
         idMap[id] = utxo;

         return utxo;
      };

      //grab utxos for scrAddrB, scrAddrC, scrAddrE
      auto promUtxo = make_shared<promise<vector<UTXO>>>();
      auto futUtxo = promUtxo->get_future();
      auto getUtxoLbd = [promUtxo](ReturnMessage<vector<UTXO>> msg)->void
      {
         promUtxo->set_value(msg.get());
      };

      mainInstance->wlt_.getSpendableTxOutListForValue(UINT64_MAX, getUtxoLbd);
      vector<UTXO> utxosB, utxosC, utxosE;
      {
         auto&& utxoVec = futUtxo.get();
         for (auto& utxo : utxoVec)
         {
            if (utxo.getRecipientScrAddr() == TestChain::scrAddrB)
               utxosB.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrC)
               utxosC.push_back(utxo);
            else if (utxo.getRecipientScrAddr() == TestChain::scrAddrE)
               utxosE.push_back(utxo);

            auto& idMap = outputMap[utxo.txHash_];
            idMap[utxo.txOutIndex_] = utxo;
         }
      }

      ASSERT_FALSE(utxosB.empty());
      ASSERT_FALSE(utxosC.empty());
      ASSERT_FALSE(utxosE.empty());

      /*create the transactions*/

      //1
      {
         //20 from B, 5 to A, change to D
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosB[0]);
         signer.addSpender(spender);

         auto recA = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 5 * COIN);
         signer.addRecipient(recA);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrD.getSliceCopy(1, 20), 
            spender->getValue() - recA->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }

      //2
      {
         auto utxoD = getUtxoFromRawTx(rawTxVec[0], 1);

         //15 from D, 10 to E, change to F
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxoD);
         signer.addSpender(spender);

         auto recE = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrE.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recE);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 
            spender->getValue() - recE->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }
      
      //3
      {
         //20 from E, 10 to F, change to A
         Signer signer;

         auto spender = make_shared<ScriptSpender>(utxosE[0]);
         signer.addSpender(spender);

         auto recF = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrF.getSliceCopy(1, 20), 10 * COIN);
         signer.addRecipient(recF);

         auto recChange = make_shared<Recipient_P2PKH>(
            TestChain::scrAddrA.getSliceCopy(1, 20), 
            spender->getValue() - recF->getValue());
         signer.addRecipient(recChange);

         signer.setFeed(feed);
         signer.sign();
         rawTxVec.push_back(signer.serializeSignedTx());
         Tx tx(rawTxVec.back());
         zcHashes.push_back(tx.getThisHash());
      }
   }

   unsigned N = 1;

   //create N side instances
   vector<shared_ptr<WSClient>> sideInstances;
   for (unsigned i=0; i<N; i++)
      sideInstances.emplace_back(setupBDV());   

   //get addresses for tx lambda
   auto getAddressesForRawTx = [&outputMap](const Tx& tx)->set<BinaryData>
   {
      set<BinaryData> addrSet;

      for (unsigned i=0; i<tx.getNumTxIn(); i++)
      {
         auto txin = tx.getTxInCopy(i);
         auto op = txin.getOutPoint();

         auto hashIter = outputMap.find(op.getTxHash());
         EXPECT_TRUE(hashIter != outputMap.end());

         auto idIter = hashIter->second.find(op.getTxOutIndex());
         EXPECT_TRUE(idIter != hashIter->second.end());

         auto& utxo = idIter->second;
         addrSet.insert(utxo.getRecipientScrAddr());
      }

      for (unsigned i=0; i<tx.getNumTxOut(); i++)
      {
         auto txout = tx.getTxOutCopy(i);
         addrSet.insert(txout.getScrAddressStr());
      }

      return addrSet;
   };

   set<BinaryData> mainScrAddrSet;
   set<BinaryData> mainHashes;   
   {
      vector<unsigned> zcIds = {1, 2};
      for (auto& id : zcIds)
      {
         Tx tx(rawTxVec[id - 1]);
         mainHashes.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         mainScrAddrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }   
   }

   //case 1
   auto case1 = [&](unsigned instanceId)->void
   {
      auto instance = sideInstances[instanceId];

      //push 1-2, 3
      vector<unsigned> zcIds = {1, 2, 3};

      vector<BinaryData> zcs;
      set<BinaryData> addrSet;
      set<BinaryData> hashSet;
      for (auto& id : zcIds)
      {
         zcs.push_back(rawTxVec[id - 1]);
         Tx tx(rawTxVec[id - 1]);
         hashSet.insert(tx.getThisHash());
         auto localAddrSet = getAddressesForRawTx(tx);
         addrSet.insert(localAddrSet.begin(), localAddrSet.end());
      }

      auto broadcastId1 = instance->bdvPtr_->broadcastZC(zcs);

      //wait on broadcast errors
      map<BinaryData, ArmoryErrorCodes> errorMap;
      errorMap.emplace(zcHashes[0], ArmoryErrorCodes::ZcBroadcast_AlreadyInMempool);
      instance->callbackPtr_->waitOnErrors(errorMap, broadcastId1);

      //wait on zc
      instance->callbackPtr_->waitOnZc(hashSet, addrSet, broadcastId1);
   };

   //main instance
   {
      //set RPC, this will allow for batches in side jobs to 
      //collide with the original one
      rpcNode_->stallNextZc(3); //in seconds

      //push 1-2

      set<BinaryData> scrAddrSet1, scrAddrSet2;
      BinaryData hash1, hash2;
         
      {
         Tx tx(rawTxVec[0]);
         hash1 = tx.getThisHash();
         scrAddrSet1 = getAddressesForRawTx(tx);
      }

      {
         Tx tx(rawTxVec[1]);
         hash2 = tx.getThisHash();
         scrAddrSet2 = getAddressesForRawTx(tx);
      }

      auto broadcastId1 = mainInstance->bdvPtr_->broadcastThroughRPC(rawTxVec[0]);
      auto broadcastId2 = mainInstance->bdvPtr_->broadcastThroughRPC(rawTxVec[1]);

      /*
      delay for 1 second before starting side jobs to make sure the 
      primary broadcast is first in line
      */
      this_thread::sleep_for(chrono::seconds(1));

      //start the side jobs
      vector<thread> threads;
      threads.push_back(thread(case1, 0));

      //wait on zc
      mainInstance->callbackPtr_->waitOnZc({hash1}, scrAddrSet1, broadcastId1);
      mainInstance->callbackPtr_->waitOnZc({hash2}, scrAddrSet2, broadcastId2);

      //wait on side jobs
      for (auto& thr : threads)
      {
         if (thr.joinable())
            thr.join();
      }

      //done
   }

   //cleanup
   auto&& bdvObj2 = AsyncClient::BlockDataViewer::getNewBDV(
      "127.0.0.1", config.listenPort_, BlockDataManagerConfig::getDataDir(),
      authPeersPassLbd_, BlockDataManagerConfig::ephemeralPeers_, nullptr);
   bdvObj2->addPublicKey(serverPubkey);
   bdvObj2->connectToRemote();

   bdvObj2->shutdown(config.cookie_);
   WebSocketServer::waitOnShutdown();

   EXPECT_EQ(theBDMt_->bdm()->zeroConfCont()->getMatcherMapSize(), 0);

   delete theBDMt_;
   theBDMt_ = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/*
Zc failure tests:
   p2p node down
   p2p timeout into rpc down
   p2p timeout into rpc successful push but client d/c in between (dangling bdvPtr)
*/

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Now actually execute all the tests
////////////////////////////////////////////////////////////////////////////////
GTEST_API_ int main(int argc, char **argv)
{
#ifdef _MSC_VER
   _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

   WSADATA wsaData;
   WORD wVersion = MAKEWORD(2, 0);
   WSAStartup(wVersion, &wsaData);
#endif

   btc_ecc_start();

   GOOGLE_PROTOBUF_VERIFY_VERSION;
   srand(time(0));
   std::cout << "Running main() from gtest_main.cc\n";

   // Setup the log file 
   STARTLOGGING("cppTestsLog.txt", LogLvlDebug2);
   //LOGDISABLESTDOUT();

   testing::InitGoogleTest(&argc, argv);
   int exitCode = RUN_ALL_TESTS();

   FLUSHLOG();
   CLEANUPLOG();
   google::protobuf::ShutdownProtobufLibrary();

   btc_ecc_stop();
   return exitCode;
}
