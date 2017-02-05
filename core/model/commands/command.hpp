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

#ifndef IROHA_COMMAND_HPP
#define IROHA_COMMAND_HPP

#include "../../service/executor.hpp"
#include "../objects/object.hpp"

namespace command {

  using object::Object;
  // Entity
  class Command {
  public:

    enum class CommandType{
      ADD,
      TRANSFER,
      UPDATE,
      REMOVE,
      BATCH,
      UNBATCH,
      CONTRACT
    };

    virtual ~Command() {}
    virtual void execute(Executor&) = 0;
    virtual CommandType getCommandName() const = 0;
    virtual std::string getHash() const = 0;
    virtual Object getObject() const = 0;
  };

};

#endif //IROHA_COMMAND_H
