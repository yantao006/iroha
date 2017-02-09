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

#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <string>
#include <iostream>
#include <thread>

#include <unordered_map>

#include "../../core/consensus/connection/connection.hpp"
#include "../../core/consensus/consensus_event.hpp"
#include "../../core/model/commands/transfer.hpp"
#include "../../core/model/commands/add.hpp"
#include "../../core/model/objects/domain.hpp"
#include "../../core/model/transaction.hpp"
#include "../../core/service/peer_service.hpp"

#include "../../core/infra/config/peer_service_with_json.hpp"


int main(int argc, char* argv[]){
    logger::setLogLevel(logger::LogLevel::DEBUG);

    std::thread receiver_thread([](){
      connection::initialize_peer();
      connection::run();
    });

    connection::addSubscriber(
      config::PeerServiceConfig::getInstance().getMyIp()
    );

    connection::receive([](const std::string& from,event::ConsensusEvent& event) {
        logger::info("connection") << "receive!";
        logger::info("sumeragi") << "received message! sig:[" << event.numberOfValidSignatures() << "]";
    });

    std::string senderPublicKey = "senderPublicKey";
    std::unordered_map<std::string, object::BaseObject> values;
    values.insert(std::make_pair("asset1", object::BaseObject("value")));
    object::Asset asset(
       "domain",
       "name",
       values
    );

    auto add = command::Add(object::Object(asset));
    event::ConsensusEvent event(transaction::Transaction(
      senderPublicKey,
      command::Command(add)
    ));
    connection::send( config::PeerServiceConfig::getInstance().getMyIp(), event);

    usleep(3);
    connection::finish();

    receiver_thread.join();
    return 0;
}
