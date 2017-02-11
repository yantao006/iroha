/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <gtest/gtest.h>
#include "../../../core/model/transaction_builder/transaction_builder.hpp"
#include "../../../core/util/exception.hpp"

using transaction::TransactionBuilder;
using type_signatures::Add;
using type_signatures::Transfer;
using type_signatures::Update;
using type_signatures::Remove;

/***************************************************************************
  Add
 ***************************************************************************/
TEST(transaction_builder, create_add) {

  {
    auto txDomain = TransactionBuilder<Add<object::Domain>>()
      .setSender("karin")
      .setDomain(object::Domain({"pubkey1", "pubkey2"}, "name"))
      .build();

    ASSERT_TRUE(txDomain.senderPublicKey == "karin");
    auto addc = txDomain.command.AsAdd();
    auto domo = addc->object.AsDomain();
    ASSERT_TRUE(domo->ownerPublicKey == std::vector<std::string>({"pubkey1", "pubkey2"}));
    ASSERT_TRUE(domo->name == "name");
  }

  {
    auto txAccount = TransactionBuilder<Add<object::Account>>()
      .setSender("karin")
      .setAccount(object::Account("pubkey", "name"))
      .build();

    ASSERT_TRUE(txAccount.senderPublicKey == "karin");
    auto addc = txAccount.command.AsAdd();
    auto acco = addc->object.AsAccount();
    ASSERT_TRUE(acco->ownerPublicKey == "pubkey");
    ASSERT_TRUE(acco->name == "name");
  }

  {
    auto value = std::unordered_map<std::string, object::BaseObject>(
      {{"key1", object::BaseObject("value")}, {"key2", object::BaseObject(123456)}}
    );
    auto txAsset = TransactionBuilder<Add<object::Asset>>()
      .setSender("karin")
      .setAsset(object::Asset("domainID", "name", value))
      .build();

    ASSERT_TRUE(txAsset.senderPublicKey == "karin");
    auto addc = txAsset.command.AsAdd();
    auto asso = addc->object.AsAsset();
    ASSERT_TRUE(asso->domain == "domainID");
    ASSERT_TRUE(asso->name == "name");
    ASSERT_TRUE(asso->value == value);
  }
  
  {
    auto txPeer = TransactionBuilder<Add<object::Peer>>()
      .setSender("karin")
      .setPeer(object::Peer("ipipip", "pubkey", 1.23))
      .build();

    ASSERT_TRUE(txPeer.senderPublicKey == "karin");
    auto addc = txPeer.command.AsAdd();
    auto acco = addc->object.AsPeer();
    ASSERT_TRUE(acco->getIP() == "ipipip");
    ASSERT_TRUE(acco->getPublicKey() == "pubkey");
    ASSERT_TRUE(acco->getTrustScore() == 1.23);
  }

  {
    auto txAsset = TransactionBuilder<Add<object::SimpleAsset>>()
      .setSender("karin")
      .setSimpleAsset(object::SimpleAsset("domain", "name", object::BaseObject(123456)))
      .build();

    ASSERT_TRUE(txAsset.senderPublicKey == "karin");
    auto addc = txAsset.command.AsAdd();
    auto acco = addc->object.AsSimpleAsset();
    ASSERT_TRUE(acco->domain == "domain");
    ASSERT_TRUE(acco->name == "name");
    ASSERT_TRUE(static_cast<int>(acco->value) == 123456);
  }

  {
    auto txMessage = TransactionBuilder<Add<object::Message>>()
      .setSender("karin")
      .setMessage(object::Message(std::string("messsss")))
      .build();

    ASSERT_TRUE(txMessage.senderPublicKey == "karin");
    auto addc = txMessage.command.AsAdd();
    auto acco = addc->object.AsMessage();
    ASSERT_TRUE(acco->text == std::string("messsss"));
  }
}

/***************************************************************************
  Transfer
 ***************************************************************************/
TEST(transaction_builder, create_transfer) {

  {
    auto txDomain = TransactionBuilder<Transfer<object::Domain>>()
      .setSender("karin")
      .setReceiverPublicKey("receiverPublicKey")
      .setDomain(object::Domain({"pubkey1", "pubkey2"}, "name"))
      .build();

    ASSERT_TRUE(txDomain.senderPublicKey == "karin");
    auto addc = txDomain.command.AsTransfer();
    auto domo = addc->object.AsDomain();
    ASSERT_TRUE(domo->ownerPublicKey == std::vector<std::string>({"pubkey1", "pubkey2"}));
    ASSERT_TRUE(domo->name == "name");
  }

  {
    auto txAccount = TransactionBuilder<Transfer<object::Account>>()
      .setSender("karin")
      .setReceiverPublicKey("receiverPublicKey")
      .setAccount(object::Account("pubkey", "name"))
      .build();

    ASSERT_TRUE(txAccount.senderPublicKey == "karin");
    auto addc = txAccount.command.AsTransfer();
    auto acco = addc->object.AsAccount();
    ASSERT_TRUE(acco->ownerPublicKey == "pubkey");
    ASSERT_TRUE(acco->name == "name");
  }

  {
    auto value = std::unordered_map<std::string, object::BaseObject>(
      {{"key1", object::BaseObject("value")}, {"key2", object::BaseObject(123456)}}
    );
    auto txAsset = TransactionBuilder<Transfer<object::Asset>>()
      .setSender("karin")
      .setReceiverPublicKey("receiverPublicKey")
      .setAsset(object::Asset("domainID", "name", value))
      .build();

    ASSERT_TRUE(txAsset.senderPublicKey == "karin");
    auto addc = txAsset.command.AsTransfer();
    auto asso = addc->object.AsAsset();
    ASSERT_TRUE(asso->domain == "domainID");
    ASSERT_TRUE(asso->name == "name");
    ASSERT_TRUE(asso->value == value);
  }
}

/***************************************************************************
  Update
 ***************************************************************************/
TEST(transaction_builder, create_update) {

  {
    auto txDomain = TransactionBuilder<Update<object::Domain>>()
      .setSender("karin")
      .setDomain(object::Domain({"pubkey1", "pubkey2"}, "name"))
      .build();

    ASSERT_TRUE(txDomain.senderPublicKey == "karin");
    auto addc = txDomain.command.AsUpdate();
    auto domo = addc->object.AsDomain();
    ASSERT_TRUE(domo->ownerPublicKey == std::vector<std::string>({"pubkey1", "pubkey2"}));
    ASSERT_TRUE(domo->name == "name");
  }

  {
    auto txAccount = TransactionBuilder<Update<object::Account>>()
      .setSender("karin")
      .setAccount(object::Account("pubkey", "name"))
      .build();

    ASSERT_TRUE(txAccount.senderPublicKey == "karin");
    auto addc = txAccount.command.AsUpdate();
    auto acco = addc->object.AsAccount();
    ASSERT_TRUE(acco->ownerPublicKey == "pubkey");
    ASSERT_TRUE(acco->name == "name");
  }

  {
    auto value = std::unordered_map<std::string, object::BaseObject>(
      {{"key1", object::BaseObject("value")}, {"key2", object::BaseObject(123456)}}
    );
    auto txAsset = TransactionBuilder<Update<object::Asset>>()
      .setSender("karin")
      .setAsset(object::Asset("domainID", "name", value))
      .build();

    ASSERT_TRUE(txAsset.senderPublicKey == "karin");
    auto addc = txAsset.command.AsUpdate();
    auto asso = addc->object.AsAsset();
    ASSERT_TRUE(asso->domain == "domainID");
    ASSERT_TRUE(asso->name == "name");
    ASSERT_TRUE(asso->value == value);
  }
}

/***************************************************************************
  Remove
 ***************************************************************************/
TEST(transaction_builder, create_remove) {

  {
    auto txDomain = TransactionBuilder<Remove<object::Domain>>()
      .setSender("karin")
      .setDomain(object::Domain({"pubkey1", "pubkey2"}, "name"))
      .build();

    ASSERT_TRUE(txDomain.senderPublicKey == "karin");
    auto addc = txDomain.command.AsRemove();
    auto domo = addc->object.AsDomain();
    ASSERT_TRUE(domo->ownerPublicKey == std::vector<std::string>({"pubkey1", "pubkey2"}));
    ASSERT_TRUE(domo->name == "name");
  }

  {
    auto txAccount = TransactionBuilder<Remove<object::Account>>()
      .setSender("karin")
      .setAccount(object::Account("pubkey", "name"))
      .build();

    ASSERT_TRUE(txAccount.senderPublicKey == "karin");
    auto addc = txAccount.command.AsRemove();
    auto acco = addc->object.AsAccount();
    ASSERT_TRUE(acco->ownerPublicKey == "pubkey");
    ASSERT_TRUE(acco->name == "name");
  }

  {
    auto value = std::unordered_map<std::string, object::BaseObject>(
      {{"key1", object::BaseObject("value")}, {"key2", object::BaseObject(123456)}}
    );
    auto txAsset = TransactionBuilder<Remove<object::Asset>>()
      .setSender("karin")
      .setAsset(object::Asset("domainID", "name", value))
      .build();

    ASSERT_TRUE(txAsset.senderPublicKey == "karin");
    auto addc = txAsset.command.AsRemove();
    auto asso = addc->object.AsAsset();
    ASSERT_TRUE(asso->domain == "domainID");
    ASSERT_TRUE(asso->name == "name");
    ASSERT_TRUE(asso->value == value);
  }
}