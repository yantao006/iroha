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

#include "../world_state_repository.hpp"
#include "transaction_repository.hpp"
#include "../../consensus/consensus_event.hpp"
#include "../../crypto/base64.hpp"
#include <string>
#include <vector>
#include <stdexcept>

namespace repository{

    namespace transaction {

        void add(const std::string &key,const iroha::Transaction& tx){
        }

        std::vector<iroha::Transaction> findAll(){
        }

        iroha::Transaction find(std::string key){
        }


        std::vector<iroha::Transaction> findByAssetName(std::string name){
        }

    };
};
