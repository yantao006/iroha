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

using transaction::TransactionBuilder;

/***************************************************************************
  Domain
 ***************************************************************************/
TEST(transaction_builder, create_domain) {
  
  const std::vector<std::string> ownerPublicKey = {"publicKey1", "publicKey2"};
  const std::string name = "My domain";

  auto domain = TransactionBuilder<object::Domain>()
    .setOwnerPublicKey(ownerPublicKey)
    .setName(name)
    .build();

  ASSERT_TRUE(ownerPublicKey.size() == domain.ownerPublicKey.size());

  for (std::size_t i=0; i<ownerPublicKey.size(); i++) {
    ASSERT_STREQ(ownerPublicKey[i].c_str(), domain.ownerPublicKey[i].c_str());
  }

  ASSERT_STREQ(name.c_str(), domain.name.c_str());
}
/*
TEST(transaction_builder, create_domain_unset_members) {
  
  const std::vector<std::string> ownerPublicKey = {"publicKey1", "publicKey2"};
  ASSERT_THROW(
    auto domain = TransactionBuilder<object::Domain>()
      .setOwnerPublicKey(ownerPublicKey)
      .build();
  );
}
*/

/***************************************************************************
  Account
 ***************************************************************************/
TEST(transaction_builder, create_account) {

  const std::string ownerPublicKey = "ownerPublicKey";
  const std::string name = "karin";

  auto account = TransactionBuilder<object::Account>()
    .setOwnerPublicKey(ownerPublicKey)
    .setName(name)
    .build();

  ASSERT_STREQ(ownerPublicKey.c_str(), account.ownerPublicKey.c_str());
  ASSERT_STREQ(name.c_str(), account.name.c_str());
}

/***************************************************************************
  Asset
 ***************************************************************************/
TEST(transaction_builder, create_asset) {

  const std::string domainId = "domainId";
  const std::string name = "karin";
  const object::AssetValueT value({{"key1", object::BaseObject("val1")}, {"key2", object::BaseObject(123.456f)}});

  auto asset = TransactionBuilder<object::Asset>()
    .setDomain(domainId)
    .setName(name)
    .setValue(value)
    .build();

  ASSERT_STREQ(domainId.c_str(), asset.domain.c_str());
  ASSERT_STREQ(name.c_str(), asset.name.c_str());
  
  for (auto&& e: value) {
    const auto eKey = e.first;
    const auto eVal = e.second;

    ASSERT_TRUE(asset.value.find(eKey) != asset.value.end());
    ASSERT_TRUE(asset.value.find(eKey)->second == eVal);
  }

  for (auto&& e: asset.value) {
    const auto eKey = e.first;
    const auto eVal = e.second;

    ASSERT_TRUE(value.find(eKey) != value.end());
    ASSERT_TRUE(value.find(eKey)->second == eVal);
  }
}


/***************************************************************************
  Message
 ***************************************************************************/
TEST(transaction_builder, create_message) {

  const std::string text = "a message";

  auto message = TransactionBuilder<object::Message>()
    .setMessage(text)
    .build();

  ASSERT_STREQ(text.c_str(), message.text.c_str());
}


/***************************************************************************
  SimpleAsset
 ***************************************************************************/
TEST(transaction_builder, create_simpleAsset) {

  const std::string domainId = "domainId";
  const std::string name = "karin";
  const object::BaseObject value(12345);

  auto simpleAsset = TransactionBuilder<object::SimpleAsset>()
    .setDomain(domainId)
    .setName(name)
    .setValue(value)
    .build();

  ASSERT_STREQ(domainId.c_str(), simpleAsset.domain.c_str());
  ASSERT_STREQ(name.c_str(), simpleAsset.name.c_str());
  ASSERT_TRUE(static_cast<int>(value) == static_cast<int>(simpleAsset.value));
}


/***************************************************************************
  Peer
 ***************************************************************************/
TEST(transaction_builder, create_peer) {

  const std::string ip = "111.111.111.111";
  const std::string publicKey = "publicKey";
  const double trustScore = 123.123456;

  auto peer = TransactionBuilder<object::Peer>()
    .setIP(ip)
    .setPublicKey(publicKey)
    .setTrustScore(trustScore)
    .build();

  ASSERT_STREQ(ip.c_str(), peer.getIP().c_str());
  ASSERT_STREQ(publicKey.c_str(), peer.getPublicKey().c_str());
  ASSERT_TRUE(trustScore == peer.getTrustScore());
}
