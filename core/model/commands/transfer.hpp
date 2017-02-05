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

#ifndef CORE_DOMAIN_TRANSFER_HPP_
#define CORE_DOMAIN_TRANSFER_HPP_

#include <string>

namespace command {

    class Transfer : public Command {
        object::Object object;
        std::string senderPublicKey;
        std::string receiverPublicKey;
      public:

        Transfer(
            Object o,
            std::string sender,
            std::string receiver
        ):
            object(o),
            senderPublicKey(sender),
            receiverPublicKey(receiver)
        {}

        CommandType getCommandName() const{
          return CommandType::TRANSFER;
        }

        std::string getHash() const{
          return "WIP:Hash";
        }

        void execute(Executor& e){
          e.execute(this);
        }

        Object getObject() const{
          return object;
        }

    };  // namespace command
};
#endif  // CORE_DOMAIN_TRANSACTIONS_TRANSFERTRANSACTION_HPP_
