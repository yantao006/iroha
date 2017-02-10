/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp

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

#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <unordered_map>
#include "../../util/logger.hpp"

#include "command.hpp"

namespace command {

    // There is kind of Currency, Asset,Domain,Account,Message and Peer. Associate SmartContract with Asset.
    Command::Command() = default;	// ctor for ValueT::null

    Command::Command(const Add& rhs):
      type(CommandValueT::add)
    {
        std::cout << EnumNamesCommandValue(type) << std::endl;
        this->type = CommandValueT::add;
        add = new Add(rhs);
    }
    Command::Command(const Batch& rhs):
      type(CommandValueT::batch)
    {
        batch = new Batch(rhs);
    }
    Command::Command(const Contract& rhs):
      type(CommandValueT::contract)
    {
        contract = new Contract(rhs);
    }
    Command::Command(const Remove& rhs):
      type(CommandValueT::remove)
    {
        remove = new Remove(rhs);
    }
    Command::Command(const Transfer& rhs):
      type(CommandValueT::transfer)
    {
        transfer = new Transfer(rhs);
    }
    Command::Command(const Unbatch& rhs):
      type(CommandValueT::unbatch)
    {
        unbatch = new Unbatch(rhs);
    }
    Command::Command(const Update& rhs):
      type(CommandValueT::update)
    {
        update = new Update(rhs);
    }

    Add*        Command::AsAdd(){
        return type == CommandValueT::add?
          add : nullptr;
    }

    Batch*      Command::AsBatch(){
      return type == CommandValueT::batch?
        batch : nullptr;
    }
    Contract*   Command::AsContract(){
      return type == CommandValueT::contract?
        contract : nullptr;
    }
    Remove*     Command::AsRemove(){
      return type == CommandValueT::remove?
        remove : nullptr;
    }
    Transfer*   Command::AsTransfer(){
      return type == CommandValueT::transfer?
        transfer : nullptr;
    }
    Unbatch*    Command::AsUnbatch(){
      return type == CommandValueT::unbatch?
        unbatch : nullptr;
    }
    Update*     Command::AsUpdate(){
      return type == CommandValueT::update?
        update : nullptr;
    }

    Object Command::getObject() const{
      switch (type) {
          case CommandValueT::add: {
              return add->object;
          }
          case CommandValueT::batch: {
              logger::fatal("model command") << "batch dose not have object";
              exit(EXIT_FAILURE);
          }
          case CommandValueT::contract: {
              return contract->object;
          }
          case CommandValueT::remove: {
              return remove->object;
          }
          case CommandValueT::transfer: {
              return transfer->object;
          }
          case CommandValueT::unbatch: {
              logger::fatal("model command") << "unbatch dose not have object";
              exit(EXIT_FAILURE);
          }
          case CommandValueT::update: {
              return update->object;
            break;
          }
          case CommandValueT::null: {
              logger::fatal("model command") << "null dose not have object";
              exit(EXIT_FAILURE);
          }

          default: {
              logger::fatal("model object") << "Unexpected ValueT: " << static_cast<std::uint8_t>(type);
              exit(EXIT_FAILURE);
          }
      }
    }

    void Command::execute(Executor& e){

    }

    CommandValueT Command::getCommandType(){
      return this->type;
    }

    std::string Command::getHash(){
      return "WIP";
    }
}
