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

#ifndef CORE_CONSENSUS_CONSENSUSEVENT_HPP_
#define CORE_CONSENSUS_CONSENSUSEVENT_HPP_

#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <algorithm>

#include "../crypto/signature.hpp"
#include "../util/logger.hpp"
#include "../model/transaction.hpp"

#include "../model/commands/add.hpp"
#include "../model/commands/transfer.hpp"
#include "../model/commands/update.hpp"

#include "../model/objects/account.hpp"
#include "../model/objects/asset.hpp"
#include "../model/objects/domain.hpp"
#include "../model/objects/message.hpp"
#include "../model/objects/peer.hpp"


namespace event {

  using namespace object;
  using transaction::Transaction;

class ConsensusEvent{

    struct eventSignature{
        std::string publicKey;
        std::string signature;

        eventSignature(
            std::string pubKey,
            std::string sig
        ):
                publicKey(pubKey),
                signature(sig)
        {}
    };

    std::vector<Transaction> _transactions;
    std::vector<eventSignature> _eventSignatures;

public:
    int order;

    std::string hash;

    Asset asset;
    Domain domain;
    Message message;
    Peer  peer;

    enum Command_type{
      ADD,
      TRANSFER,
      UPDATE,
      REMOVE
    };

    ConsensusEvent(
        Transaction tx
    )
    {
      _transactions.push_back(tx)
    }

    void addEventSignature(std::string&& publicKey,std::string&& signature){
        _eventSignatures.push_back(eventSignature(publicKey, signature));
    }

    std::vetor<Transaction> transactions() const{
      return _transactions;
    }

    std::vector<EventSignature> eventSignatures() const{
        return _eventSignatures;
    };

    unsigned int numberOfValidSignatures(){
        return std::count_if(
            _eventSignatures.begin(), _eventSignatures.end(),
            [&](eventSignature sig){
                return signature::verify(sig.signature, hash, sig.publicKey);
            }
        );
    }

    std::string getTransactionHash(){
      return hash;
    }

    bool eventSignatureIsEmpty() {
        return _eventSignatures.empty();
    }

    void execution(){
    }

};
};  // namespace ConsensusEvent

#endif  // CORE_CONSENSUS_CONSENSUSEVENT_HPP_
