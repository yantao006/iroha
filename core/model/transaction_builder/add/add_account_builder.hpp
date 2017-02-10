/*
Copyright 2016 Soramitsu Co., Ltd.

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
#include "../../commands/add.hpp"
#include "../../objects/account.hpp"

template <>
class TransactionBuilder<Add<object::Account>> {
 public:
  TransactionBuilder() = default;
  TransactionBuilder(const TransactionBuilder&) = default;
  TransactionBuilder(TransactionBuilder&&) = default;
  TransactionBuilder& setSender(std::string sender) {
    _sender = std::move(sender);
    return *this;
  }
  TransactionBuilder& setAccount(object::Account object) {
    _object = std::move(object);
    return *this;
  }
  object::Account build() {
    const auto unsetMembers = enumerateUnsetMembers();
    if (not unsetMembers.empty()) {
      throw exception::transaction::UnsetBuildArgmentsException(
          "Add<object::Account>", unsetMembers);
    }
    return _object;
  }

 private:
  std::string enumerateUnsetMembers() {
    std::string ret;
    if (_sender.empty()) ret += " " + "sender";
    if (_object.empty()) ret += " " + "object";
    return ret;
  }
  std::string _sender;
  object::Account _object;
};
