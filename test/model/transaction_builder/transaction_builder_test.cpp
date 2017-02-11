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

/***************************************************************************
  Add
 ***************************************************************************/
TEST(transaction_builder, create_add) {

  auto txDomain = TransactionBuilder<Add<object::Domain>>()
    .setSender("karin")
    .setDomain(object::Domain({"pubkey1", "pubkey2"}, "name"))
    .build();

  auto addc = txDomain.command.AsAdd();
  auto domo = addc->object.AsDomain();
  ASSERT_TRUE(domo->ownerPublicKey == std::vector<std::string>({"pubkey1", "pubkey2"}));
  
/*
  auto txAccount = TransactionBuilder<Add<object::Account>>()
    .setSender("karin")
    .setAccount(object::Account("pubkey", "name"))
    .build();

  auto txAsset = TransactionBuilder<Add<object::Asset>>()
    .setSender("karin")
    .setAsset(object::Asset("domainID", "name", {
      {"key1", object::BaseObject("value")},
      {"key2", object::BaseObject(123456)}
    }))
    .build();
*/

/*
  auto domain = object::Domain("");
  auto tx = TransactionBuilder<Add<object::Domain, To<object::Domain>>()
    .setSender("nao")
    .setDomain()
*/
}
/*
TEST(transaction_builder, create_update) {

}

TEST(transaction_builder, create_remove) {

}

TEST(transaction_builder, create_remove) {

}
*/