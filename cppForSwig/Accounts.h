////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Copyright (C) 2017-2019, goatpig                                          //
//  Distributed under the MIT license                                         //
//  See LICENSE-MIT or https://opensource.org/licenses/MIT                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

#ifndef _H_ACCOUNTS
#define _H_ACCOUNTS

#include <set>
#include <map>
#include <memory>

#include "WalletFileInterface.h"

#include "ReentrantLock.h"
#include "BinaryData.h"
#include "EncryptionUtils.h"
#include "Assets.h"
#include "Addresses.h"
#include "DerivationScheme.h"

#define ARMORY_LEGACY_ACCOUNTID        0xF6E10000
#define IMPORTS_ACCOUNTID              0x00000000
#define ARMORY_LEGACY_ASSET_ACCOUNTID  0x00000001

#define BIP32_OUTER_ACCOUNT_DERIVATIONID 0x00000000
#define BIP32_INNER_ACCOUNT_DERIVATIONID 0x00000001

#define ECDH_ASSET_ACCOUTID 0x20000000

#define ADDRESS_ACCOUNT_PREFIX   0xD0
#define ASSET_ACCOUNT_PREFIX     0xE1
#define ASSET_COUNT_PREFIX       0xE2
#define ASSET_TOP_INDEX_PREFIX   0xE3

#define META_ACCOUNT_COMMENTS    0x000000C0
#define META_ACCOUNT_AUTHPEER    0x000000C1
#define META_ACCOUNT_PREFIX      0xF1

class AccountException : public std::runtime_error
{
public:
   AccountException(const std::string& err) : std::runtime_error(err)
   {}
};

struct UnrequestedAddressException
{};

enum AssetAccountTypeEnum
{
   AssetAccountTypeEnum_Plain = 0,
   AssetAccountTypeEnum_ECDH
};

enum AccountTypeEnum
{
   /*
   armory derivation scheme 
   outer and inner account are the same
   uncompressed P2PKH, compresed P2SH-P2PK, P2SH-P2WPKH
   */
   AccountTypeEnum_ArmoryLegacy = 0,

   /*
   BIP32 derivation scheme, derPath is used as is.
   no address type is assumed, this has to be provided at creation
   */
   AccountTypeEnum_BIP32,

   /*
   Derives from BIP32_Custom, ECDH all keys pairs with salt, 
   carried by derScheme object.
   */
   AccountTypeEnum_BIP32_Salted,

   /*
   Stealth address account. Has a single key pair, ECDH it with custom
   salts per asset.
   */
   AccountTypeEnum_ECDH,

   AccountTypeEnum_Custom
};

enum MetaAccountType
{
   MetaAccount_Unset = 0,
   MetaAccount_Comments,
   MetaAccount_AuthPeers
};

namespace ArmorySigner
{
   class BIP32_AssetPath;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
struct AccountType
{
protected:
   std::set<AddressEntryType> addressTypes_;
   AddressEntryType defaultAddressEntryType_;
   bool isMain_ = false;

public:
   //tors
   AccountType()
   {}

   virtual ~AccountType() = 0;

   //locals
   void setMain(bool ismain) { isMain_ = ismain; }
   const bool isMain(void) const { return isMain_; }

   const std::set<AddressEntryType>& getAddressTypes(void) const 
   { return addressTypes_; }

   AddressEntryType getDefaultAddressEntryType(void) const 
   { return defaultAddressEntryType_; }

   void setAddressTypes(const std::set<AddressEntryType>&);
   void setDefaultAddressType(AddressEntryType);

   //virtuals
   virtual AccountTypeEnum type(void) const = 0;
   virtual BinaryData getAccountID(void) const = 0;
   virtual BinaryData getOuterAccountID(void) const = 0;
   virtual BinaryData getInnerAccountID(void) const = 0;
   virtual bool isWatchingOnly(void) const = 0;
};

////////////////////
struct AccountType_WithRoot : public AccountType
{
protected:
   SecureBinaryData privateRoot_;
   SecureBinaryData publicRoot_;
   mutable SecureBinaryData chainCode_;

protected:
   AccountType_WithRoot()
   {}

public:
   AccountType_WithRoot(
      SecureBinaryData& privateRoot,
      SecureBinaryData& publicRoot,
      SecureBinaryData& chainCode) : 
      privateRoot_(std::move(privateRoot)),
      publicRoot_(std::move(publicRoot)),
      chainCode_(std::move(chainCode))
   {
      if (privateRoot_.getSize() == 0 && publicRoot_.getSize() == 0)
         throw AccountException("need at least one valid root");

      if (privateRoot_.getSize() > 0 && publicRoot_.getSize() > 0)
         throw AccountException("root types are mutualy exclusive");

      if (publicRoot_.getSize() > 0 && chainCode_.getSize() == 0)
         throw AccountException("need chaincode for public account");
   }

   //virtuals
   virtual ~AccountType_WithRoot(void) = 0;
   virtual const SecureBinaryData& getChaincode(void) const = 0;
   virtual const SecureBinaryData& getPrivateRoot(void) const = 0;
   virtual const SecureBinaryData& getPublicRoot(void) const = 0;
   
   bool isWatchingOnly(void) const override
   {
      return privateRoot_.empty() &&
         !publicRoot_.empty() && !chainCode_.empty();
   }
};

////////////////////
struct AccountType_ArmoryLegacy : public AccountType_WithRoot
{
public:
   AccountType_ArmoryLegacy(
      SecureBinaryData& privateRoot,
      SecureBinaryData& publicRoot,
      SecureBinaryData& chainCode) :
      AccountType_WithRoot(
      privateRoot, publicRoot, chainCode)
   {
      //uncompressed p2pkh
      addressTypes_.insert(AddressEntryType(
         AddressEntryType_P2PKH | AddressEntryType_Uncompressed));

      //nested compressed p2pk
      addressTypes_.insert(AddressEntryType(
         AddressEntryType_P2PK | AddressEntryType_P2SH));

      //nested p2wpkh
      addressTypes_.insert(AddressEntryType(
         AddressEntryType_P2WPKH | AddressEntryType_P2SH));

      //native p2wpkh
      addressTypes_.insert(AddressEntryType_P2WPKH);

      //default type
      defaultAddressEntryType_ = AddressEntryType(
         AddressEntryType_P2PKH | AddressEntryType_Uncompressed);
   }

   AccountTypeEnum type(void) const
   { return AccountTypeEnum_ArmoryLegacy; }

   const SecureBinaryData& getChaincode(void) const;
   const SecureBinaryData& getPrivateRoot(void) const { return privateRoot_; }
   const SecureBinaryData& getPublicRoot(void) const { return publicRoot_; }
   BinaryData getAccountID(void) const { return WRITE_UINT32_BE(ARMORY_LEGACY_ACCOUNTID); }
   BinaryData getOuterAccountID(void) const;
   BinaryData getInnerAccountID(void) const;
};

////////////////////
struct AccountType_BIP32 : public AccountType_WithRoot
{   
   friend struct AccountType_BIP32_Custom;
   friend class AssetWallet_Single;

private:
   std::vector<uint32_t> derivationPath_;
   unsigned depth_ = 0;
   unsigned leafId_ = 0;

   std::set<unsigned> nodes_;   
   BinaryData outerAccount_;
   BinaryData innerAccount_;

protected:
   unsigned fingerPrint_ = 0;
   unsigned seedFingerprint_ = UINT32_MAX;
   unsigned addressLookup_ = UINT32_MAX;

protected:
   void setPrivateKey(const SecureBinaryData&);
   void setPublicKey(const SecureBinaryData&);
   void setChaincode(const SecureBinaryData&);
   void setDerivationPath(std::vector<unsigned> derPath)
   {
      derivationPath_ = derPath;
   }

   void setSeedFingerprint(unsigned fingerprint) 
   { 
      seedFingerprint_ = fingerprint; 
   }

   void setFingerprint(unsigned fingerprint)
   {
      fingerPrint_ = fingerprint;
   }

   void setDepth(unsigned depth)
   {
      depth_ = depth;
   }

   void setLeafId(unsigned leafid)
   {
      leafId_ = leafid;
   }

public:
   AccountType_BIP32(const std::vector<unsigned>& derivationPath) :
      AccountType_WithRoot(), derivationPath_(derivationPath)
   {}

   //bip32 virtuals
   AccountType_BIP32(void) {}
   virtual std::set<unsigned> getNodes(void) const
   {
      return nodes_;
   }

   //AccountType virtuals
   const SecureBinaryData& getChaincode(void) const override
   {
      return chainCode_;
   }

   const SecureBinaryData& getPrivateRoot(void) const override
   {
      return privateRoot_;
   }

   const SecureBinaryData& getPublicRoot(void) const override
   {
      return publicRoot_;
   }

   BinaryData getAccountID(void) const override;

   //bip32 locals
   unsigned getDepth(void) const { return depth_; }
   unsigned getLeafID(void) const { return leafId_; }
   unsigned getFingerPrint(void) const { return fingerPrint_; }
   std::vector<uint32_t> getDerivationPath(void) const 
   { return derivationPath_; }

   unsigned getSeedFingerprint(void) const { return seedFingerprint_; }

   unsigned getAddressLookup(void) const;
   void setAddressLookup(unsigned count) 
   { 
      addressLookup_ = count; 
   }

   void setNodes(const std::set<unsigned>& nodes);
   void setOuterAccountID(const BinaryData&);
   void setInnerAccountID(const BinaryData&);

   virtual AccountTypeEnum type(void) const override
   { return AccountTypeEnum_BIP32; }

   BinaryData getOuterAccountID(void) const override;
   BinaryData getInnerAccountID(void) const override;

   void addAddressType(AddressEntryType);
   void setDefaultAddressType(AddressEntryType);
};

////////////////////////////////////////////////////////////////////////////////
struct AccountType_BIP32_Salted : public AccountType_BIP32
{
private:
   const SecureBinaryData salt_;

public:
   AccountType_BIP32_Salted(
      const std::vector<unsigned>& derivationPath,
      const SecureBinaryData& salt) :
      AccountType_BIP32(derivationPath), salt_(salt)
   {}

   AccountTypeEnum type(void) const 
   { return AccountTypeEnum_BIP32_Salted; }

   const SecureBinaryData& getSalt(void) const 
   { return salt_; }
};

////////////////////////////////////////////////////////////////////////////////
class AccountType_ECDH : public AccountType
{
private:
   const SecureBinaryData privateKey_;
   const SecureBinaryData publicKey_;

   //ECDH accounts are always single
   const BinaryData accountID_;

public:
   //tor
   AccountType_ECDH(
      const SecureBinaryData& privKey,
      const SecureBinaryData& pubKey) :
      privateKey_(privKey), publicKey_(pubKey),
      accountID_(WRITE_UINT32_BE(ECDH_ASSET_ACCOUTID))
   {
      //run checks
      if (privateKey_.getSize() == 0 && publicKey_.getSize() == 0)
         throw AccountException("invalid key length");
   }

   //local
   const SecureBinaryData& getPrivKey(void) const { return privateKey_; }
   const SecureBinaryData& getPubKey(void) const { return publicKey_; }

   //virtual
   AccountTypeEnum type(void) const override { return AccountTypeEnum_ECDH; }
   BinaryData getAccountID(void) const override;
   BinaryData getOuterAccountID(void) const override { return accountID_; }
   BinaryData getInnerAccountID(void) const override { return accountID_; }
   virtual bool isWatchingOnly(void) const override;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class AssetAccount : protected Lockable
{
   friend class AssetAccount_ECDH;
   friend class AddressAccount;

private:
   BinaryData id_;
   BinaryData parent_id_;

   std::shared_ptr<AssetEntry> root_;
   std::shared_ptr<DerivationScheme> derScheme_;

   std::shared_ptr<WalletDBInterface> iface_;
   const std::string dbName_;

   std::map<unsigned, std::shared_ptr<AssetEntry>> assets_;
   unsigned lastUsedIndex_ = UINT32_MAX;

   //<assetID, <address type, prefixed address hash>>
   std::map<BinaryData, std::map<AddressEntryType, BinaryData>> addrHashMap_;
   unsigned lastHashedAsset_ = UINT32_MAX;

private:
   size_t writeAssetEntry(std::shared_ptr<AssetEntry>);
   void updateOnDiskAssets(void);

   void updateHighestUsedIndex(void);
   unsigned getAndBumpHighestUsedIndex(void);

   void commit(void);
   void updateAssetCount(void);

   void extendPublicChainToIndex(unsigned);
   void extendPublicChain(std::shared_ptr<AssetEntry>, unsigned);
   std::vector<std::shared_ptr<AssetEntry>> extendPublicChain(
      std::shared_ptr<AssetEntry>, unsigned, unsigned);

   void extendPrivateChain(
      std::shared_ptr<DecryptedDataContainer>,
      unsigned);
   void extendPrivateChainToIndex(
      std::shared_ptr<DecryptedDataContainer>,
      unsigned);
   void extendPrivateChain(
      std::shared_ptr<DecryptedDataContainer>,
      std::shared_ptr<AssetEntry>, unsigned);
   std::vector<std::shared_ptr<AssetEntry>> extendPrivateChain(
      std::shared_ptr<DecryptedDataContainer>,
      std::shared_ptr<AssetEntry>,
      unsigned, unsigned);

   std::shared_ptr<AssetEntry> getNewAsset(void);
   std::shared_ptr<AssetEntry> peekNextAsset(void);
   std::shared_ptr<Asset_PrivateKey> fillPrivateKey(
      std::shared_ptr<DecryptedDataContainer> ddc,
      const BinaryData& id);

   virtual unsigned getLookup(void) const { return DERIVATION_LOOKUP; }
   virtual uint8_t type(void) const { return AssetAccountTypeEnum_Plain; }

public:
   AssetAccount(
      const BinaryData& ID,
      const BinaryData& parentID,
      std::shared_ptr<AssetEntry> root,
      std::shared_ptr<DerivationScheme> scheme,
      std::shared_ptr<WalletDBInterface> iface, 
      const std::string& dbName) :
      id_(ID), parent_id_(parentID),
      root_(root), derScheme_(scheme),
      iface_(iface), dbName_(dbName)
   {}

   size_t getAssetCount(void) const { return assets_.size(); }
   int getLastComputedIndex(void) const;
   unsigned getHighestUsedIndex(void) const { return lastUsedIndex_; }
   std::shared_ptr<AssetEntry> getLastAssetWithPrivateKey(void) const;

   std::shared_ptr<AssetEntry> getAssetForID(const BinaryData&) const;
   std::shared_ptr<AssetEntry> getAssetForIndex(unsigned id) const;

   void updateAddressHashMap(const std::set<AddressEntryType>&);
   const std::map<BinaryData, std::map<AddressEntryType, BinaryData>>&
      getAddressHashMap(const std::set<AddressEntryType>&);

   const BinaryData& getID(void) const { return id_; }
   BinaryData getFullID(void) const { return parent_id_ + id_; }
   const SecureBinaryData& getChaincode(void) const;

   void extendPublicChain(unsigned);

   //static
   static void putData(LMDB* db, const BinaryData& key, const BinaryData& data);
   static std::shared_ptr<AssetAccount> loadFromDisk(const BinaryData& key, 
      std::shared_ptr<WalletDBInterface>, const std::string&);

   //Lockable virtuals
   void initAfterLock(void) {}
   void cleanUpBeforeUnlock(void) {}

   std::shared_ptr<AssetEntry> getRoot(void) const { return root_; }
};

////////////////////////////////////////////////////////////////////////////////
class AssetAccount_ECDH : public AssetAccount
{
private:
   unsigned getLookup(void) const override { return 1; }
   uint8_t type(void) const override { return AssetAccountTypeEnum_ECDH; }

public:
   AssetAccount_ECDH(
      const BinaryData& ID,
      const BinaryData& parentID,
      std::shared_ptr<AssetEntry> root,
      std::shared_ptr<DerivationScheme> scheme,
      std::shared_ptr<WalletDBInterface> iface, 
      const std::string& dbName) :
      AssetAccount(ID, parentID, root, scheme, iface, dbName)
   {}

   unsigned addSalt(const SecureBinaryData&);
   unsigned getSaltIndex(const SecureBinaryData&) const;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class AddressAccount : public Lockable
{
   friend class AssetWallet;
   friend class AssetWallet_Single;

private:
   std::map<BinaryData, std::shared_ptr<AssetAccount>> assetAccounts_;
   std::map<BinaryData, AddressEntryType> addresses_;

   BinaryData outerAccount_;
   BinaryData innerAccount_;

   AddressEntryType defaultAddressEntryType_ = AddressEntryType_P2PKH;
   std::set<AddressEntryType> addressTypes_;

   //<prefixed address hash, <assetID, address type>>
   std::map<BinaryData, std::pair<BinaryData, AddressEntryType>> addressHashes_;

   //account id, asset id
   std::map<BinaryData, BinaryData> topHashedAssetId_;

   BinaryData ID_;

   std::shared_ptr<WalletDBInterface> iface_;
   const std::string dbName_;

private:
   void commit(void); //used for initial commit to disk
   void reset(void);

   void addAccount(std::shared_ptr<AssetAccount>);

   void updateInstantiatedAddressType(std::shared_ptr<AddressEntry>);
   void updateInstantiatedAddressType(
      const BinaryData&, AddressEntryType);
   void writeAddressType(const BinaryData&, AddressEntryType);
   void eraseInstantiatedAddressType(const BinaryData&);
   std::shared_ptr<Asset_PrivateKey> fillPrivateKey(
      std::shared_ptr<DecryptedDataContainer> ddc,
      const BinaryData& id);

   std::shared_ptr<AssetEntry_BIP32Root> getBip32RootForAssetId(
      const BinaryData&) const;

public:
   AddressAccount(std::shared_ptr<WalletDBInterface> iface,
      const std::string& dbName) :
      iface_(iface), dbName_(dbName)
   {}

   const BinaryData& getID(void) const { return ID_; }

   void make_new(
      std::shared_ptr<AccountType>,
      std::shared_ptr<DecryptedDataContainer>,
      std::unique_ptr<Cipher>);

   void readFromDisk(const BinaryData& key);

   void extendPublicChain(unsigned);
   void extendPublicChainToIndex(const BinaryData&, unsigned);

   void extendPrivateChain(
      std::shared_ptr<DecryptedDataContainer>,
      unsigned);
   void extendPrivateChainToIndex(
      std::shared_ptr<DecryptedDataContainer>,
      const BinaryData&, unsigned);

   std::shared_ptr<AddressEntry> getNewAddress(
      AddressEntryType aeType = AddressEntryType_Default);
   std::shared_ptr<AddressEntry> getNewChangeAddress(
      AddressEntryType aeType = AddressEntryType_Default);
   std::shared_ptr<AddressEntry> peekNextChangeAddress(
      AddressEntryType aeType = AddressEntryType_Default);
   std::shared_ptr<AddressEntry> getNewAddress(
      const BinaryData& account, AddressEntryType aeType);
   std::shared_ptr<AssetEntry> getOutterAssetForIndex(unsigned) const;
   std::shared_ptr<AssetEntry> getOutterAssetRoot(void) const;


   AddressEntryType getAddressType(void) const
      { return defaultAddressEntryType_; }
   std::set<AddressEntryType> getAddressTypeSet(void) const
      { return addressTypes_; }
   bool hasAddressType(AddressEntryType);

   //get asset by binary string ID
   std::shared_ptr<AssetEntry> getAssetForID(const BinaryData&) const;

   //get asset by integer ID; bool arg defines whether it comes from the
   //outer account (true) or the inner account (false)
   std::shared_ptr<AssetEntry> getAssetForID(unsigned, bool) const;

   const std::pair<BinaryData, AddressEntryType>& 
      getAssetIDPairForAddr(const BinaryData&);
   const std::pair<BinaryData, AddressEntryType>& 
      getAssetIDPairForAddrUnprefixed(const BinaryData&);

   void updateAddressHashMap(void);
   const std::map<BinaryData, std::pair<BinaryData, AddressEntryType>>& 
      getAddressHashMap(void);

   std::shared_ptr<AssetAccount> getOuterAccount(void) const;
   const std::map<BinaryData, std::shared_ptr<AssetAccount>>& 
      getAccountMap(void) const;

   const BinaryData& getOuterAccountID(void) const { return outerAccount_; }
   const BinaryData& getInnerAccountID(void) const { return innerAccount_; }

   std::shared_ptr<AddressAccount> getWatchingOnlyCopy(
      std::shared_ptr<WalletDBInterface>, const std::string&) const;

   std::shared_ptr<AddressEntry> getAddressEntryForID(
      const BinaryDataRef&) const;
   std::map<BinaryData, std::shared_ptr<AddressEntry>> 
      getUsedAddressMap(void) const;

   //Lockable virtuals
   void initAfterLock(void) {}
   void cleanUpBeforeUnlock(void) {}

   bool hasBip32Path(const ArmorySigner::BIP32_AssetPath&) const;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
class MetaDataAccount : public Lockable
{
   friend struct AuthPeerAssetConversion;
   friend struct CommentAssetConversion;

private:
   MetaAccountType type_ = MetaAccount_Unset;
   BinaryData ID_;

   std::shared_ptr<WalletDBInterface> iface_;
   const std::string dbName_;

   std::map<unsigned, std::shared_ptr<MetaData>> assets_;

private:
   bool writeAssetToDisk(std::shared_ptr<MetaData>);

public:
   MetaDataAccount(std::shared_ptr<WalletDBInterface> iface,
      const std::string& dbName) :
      iface_(iface), dbName_(dbName)
   {}

   //Lockable virtuals
   void initAfterLock(void) {}
   void cleanUpBeforeUnlock(void) {}

   //storage methods
   void readFromDisk(const BinaryData& key);
   void commit(void);
   void updateOnDisk(void);
   std::shared_ptr<MetaDataAccount> copy(
      std::shared_ptr<WalletDBInterface>, const std::string&) const;

   //setup methods
   void reset(void);
   void make_new(MetaAccountType);

   //
   std::shared_ptr<MetaData> getMetaDataByIndex(unsigned) const;
   void eraseMetaDataByIndex(unsigned);
   MetaAccountType getType(void) const { return type_; }
};

struct AuthPeerAssetMap
{
   //<name, authorized pubkey>
   std::map<std::string, const SecureBinaryData*> nameKeyPair_;
   
   //<pubkey, sig>
   std::pair<SecureBinaryData, SecureBinaryData> rootSignature_;

   //<pubkey, description>
   std::map<SecureBinaryData, std::pair<std::string, unsigned>> peerRootKeys_;
};

////////////////////////////////////////////////////////////////////////////////
struct AuthPeerAssetConversion
{
   static AuthPeerAssetMap getAssetMap(
      const MetaDataAccount*);
   static std::map<SecureBinaryData, std::set<unsigned>> getKeyIndexMap(
      const MetaDataAccount*);

   static int addAsset(MetaDataAccount*, const SecureBinaryData&,
      const std::vector<std::string>&);

   static void addRootSignature(MetaDataAccount*, 
      const SecureBinaryData&, const SecureBinaryData&);
   static unsigned addRootPeer(MetaDataAccount*,
      const SecureBinaryData&, const std::string&);
};

////////////////////////////////////////////////////////////////////////////////
struct CommentAssetConversion
{
   static std::shared_ptr<CommentData> getByKey(MetaDataAccount*,
      const BinaryData&);

   static int setAsset(MetaDataAccount*, const BinaryData&,
      const std::string&);

   static int deleteAsset(MetaDataAccount*, const BinaryData&);

   static std::map<BinaryData, std::string> getCommentMap(MetaDataAccount*);
};
#endif