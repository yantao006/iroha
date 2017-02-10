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
  domain
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
  account
 ***************************************************************************/
  /*
TEST(transaction_builder, create_account) {

  const std::vector<std::string> ownerPublicKey = "ownerPublicKey";
  const std::string name = "karin";

  auto account = TransactionBuilder<object::Account>()
    .setOwnerPublicKey(ownerPublicKey)
    .setName(name)
    .build();

  ASSERT_TRUE(ownerPublicKey.size() == domain.ownerPublicKey.size());

  for (std::size_t i=0; i<ownerPublicKey.size(); i++) {
    ASSERT_STREQ(ownerPublicKey[i].c_str(), domain.ownerPublicKey[i].c_str());
  }

  ASSERT_STREQ(domainName.c_str(), domain.name.c_str());
}
*/
