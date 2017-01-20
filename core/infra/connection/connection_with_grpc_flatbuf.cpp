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

#include "../../consensus/connection/connection.hpp"
#include "../../util/logger.hpp"
#include "../../service/peer_service.hpp"

#include <thread>

#include <grpc++/grpc++.h>

#include "../../../flatbuf/api_generated.h"
#include "../../../flatbuf/api.grpc.fb.h"

namespace connection {
    using namespace iroha;

    std::vector<std::string> receiver_ips;
    std::vector<
        std::function<void(
            const std::string& from,
            ConsensusEvent& message)>
    > receivers;

    template<typename T>
    flatbuffers::Offset<T> makeOffsset(const T* o);

    template<>
    flatbuffers::Offset<iroha::Contract> makeOffsset<iroha::Contract>(const iroha::Contract* contract){
      flatbuffers::FlatBufferBuilder fbb;
      fbb.Clear();

      flatbuffers::Offset<void> object;
      switch(contract->object_type()){
        case Object::Object_SimpleAsset:
             object = makeOffsset(reinterpret_cast<const SimpleAsset *>(contract->object())).Union(); break;
        case Object::Object_Asset:
             object = makeOffsset(reinterpret_cast<const Asset *>(contract->object())).Union(); break;
        case Object::Object_Domain:
             object = makeOffsset(reinterpret_cast<const Domain *>(contract->object())).Union(); break;
        case Object::Object_Account:
             object = makeOffsset(reinterpret_cast<const Account *>(contract->object())).Union(); break;
        case Object::Object_Peer:
             object = makeOffsset(reinterpret_cast<const Peer *>(contract->object())).Union(); break;
      }
      return CreateContract(
        fbb,
        contract->object_type(),
        object,
        fbb.CreateString(contract->command()),
        fbb.CreateString(contract->contractName())
      );
    }

    template<>
    flatbuffers::Offset<iroha::TxSignature> makeOffsset<iroha::TxSignature>(const iroha::TxSignature* txSig){
      flatbuffers::FlatBufferBuilder fbb;
      fbb.Clear();
      return CreateTxSignature(
          fbb,
          fbb.CreateString(txSig->publicKey()),
          fbb.CreateString(txSig->signature())
      );
    }
    template<>
    flatbuffers::Offset<iroha::Transaction> makeOffsset<iroha::Transaction>(const iroha::Transaction* tx){
      flatbuffers::FlatBufferBuilder fbb;
      fbb.Clear();

      std::vector<flatbuffers::Offset<iroha::TxSignature>> txSigs;
      for(
        auto txs = tx->txSignatures()->begin();
        txs != tx->txSignatures()->end();
        txs++
      ) {
        txSigs.push_back( makeOffsset<iroha::TxSignature>( *txs ));
      }

      flatbuffers::Offset<void> command;
      switch(tx->command_type()){
        case Command::Command_Add:
             command = makeOffsset(reinterpret_cast<const Add *>(tx->command())).Union(); break;
        case Command::Command_Transfer:
             command = makeOffsset(reinterpret_cast<const Transfer *>(tx->command())).Union(); break;
        case Command::Command_Update:
             command = makeOffsset(reinterpret_cast<const Update *>(tx->command())).Union(); break;
        case Command::Command_Remove:
             command = makeOffsset(reinterpret_cast<const Remove *>(tx->command())).Union(); break;
        case Command::Command_Batch:
             command = makeOffsset(reinterpret_cast<const Batch *>(tx->command())).Union(); break;
        case Command::Command_Unbatch:
             command = makeOffsset(reinterpret_cast<const Unbatch *>(tx->command())).Union(); break;
        case Command::Command_Contract:
             command = makeOffsset(reinterpret_cast<const Contract *>(tx->command())).Union(); break;
      }
      return CreateTransaction(
        fbb,
        fbb.CreateString(tx->sender()),
        tx->command_type(),
        command,
        fbb.CreateVector(txSigs),
        fbb.CreateString(tx->hash())
      );
    }
    template<>
    flatbuffers::Offset<iroha::EventSignature> makeOffsset<iroha::EventSignature>(const iroha::EventSignature* eventSig){
      flatbuffers::FlatBufferBuilder fbb;
      fbb.Clear();
      return CreateEventSignature(
          fbb,
          fbb.CreateString(eventSig->publicKey()),
          fbb.CreateString(eventSig->signature())
      );
    }
    template<>
    flatbuffers::Offset<iroha::ConsensusEvent> makeOffsset<iroha::ConsensusEvent>(const iroha::ConsensusEvent* event){
      flatbuffers::FlatBufferBuilder fbb;
      fbb.Clear();
      std::vector<flatbuffers::Offset<iroha::Transaction>> txv;
      for(
        auto tx = event->transaction()->begin();
        tx != event->transaction()->end();
        tx++
      ) {
        txv.push_back( makeOffsset<iroha::Transaction>( *tx ));
      }

      std::vector<flatbuffers::Offset<iroha::EventSignature>> eSigs;
      for(
        auto esig = event->eventSignatures()->begin();
        esig != event->eventSignatures()->end();
        esig++
      ) {
        eSigs.push_back( makeOffsset<iroha::EventSignature>( *esig ));
      }

      return CreateConsensusEvent(
          fbb,
          fbb.CreateVector(txv),
          fbb.CreateVector(eSigs),
          event->state()
      );
    }


    class IrohaServiceImpl final : public Iroha::Service {
        flatbuffers::FlatBufferBuilder fbb_;
    public:
        virtual ::grpc::Status Torii(
          ::grpc::ServerContext* context,
          const flatbuffers::BufferRef<ConsensusEvent> *consensusEvent,
          flatbuffers::BufferRef<Response> *response
        ) override {

            fbb_.Clear();

            logger::info("connection")  <<  "Operation";
            logger::info("connection")  <<  "eventSignatures size: "    <<  consensusEvent->GetRoot()->eventSignatures()->size();


            flatbuffers::FlatBufferBuilder bb;
            ConsensusEvent user = consensusEvent.getRootAsConsensusEvent(bb); 

            for (auto& f: receivers){
                //f("dummy", event);
            }

            fbb_.Clear();
            fbb_.Finish(CreateResponse(
                fbb_,
                200,
        	      fbb_.CreateString("Ok")
            ));

            *response = flatbuffers::BufferRef<Response>(
                fbb_.GetBufferPointer(),
                fbb_.GetSize()
            );
            return grpc::Status::OK;
        }
    };

    grpc::Server *server_instance = nullptr;
    std::mutex wait_for_server;
    std::condition_variable server_instance_cv;
    IrohaServiceImpl service;
    grpc::ServerBuilder builder;

    void initialize_peer() {
        std::string server_address("0.0.0.0:50051");
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        builder.RegisterService(&service);
    }

    bool send(
        const std::string& ip,
        const ConsensusEvent& event
    ) {
        logger::info("connection") << "start send";
        if (find(receiver_ips.begin(), receiver_ips.end(), ip) != receiver_ips.end()){
            logger::info("connection")  <<  "create client";
            auto channel = grpc::CreateChannel(
              "localhost:50051",
              grpc::InsecureChannelCredentials()
            );
            auto stub = Iroha::NewStub(channel);
            grpc::ClientContext context;

            flatbuffers::FlatBufferBuilder fbb;
            fbb.Finish(makeOffsset(&event));

            auto request = flatbuffers::BufferRef<ConsensusEvent>(
              fbb.GetBufferPointer(),
              fbb.GetSize()
            );
            flatbuffers::BufferRef<Response> response;

            auto status = stub->Torii(&context, request, &response);

            if (status.ok()) {
              auto message = response.GetRoot()->message();
              logger::info("connection sumeragi's response: ") << message->str();
            } else {
              logger::error("connection sumeragi response error");
            }
            return true;
        }else{
            logger::error("connection") <<  "not found";
            return false;
        }
    }

    bool sendAll(
        const Event::ConsensusEvent& event
    ) {
        for (auto& ip : receiver_ips){
            if (ip != peer::getMyIp()){
                send( ip, event);
            }
        }
        return true;
    }

    bool receive(const std::function<void(
        const std::string&,
        ConsensusEvent&)>& callback) {
        receivers.push_back(callback);
        return true;
    }

    void addSubscriber(std::string ip) {
        receiver_ips.push_back(ip);
    }

    int run() {
      wait_for_server.lock();
      server_instance = builder.BuildAndStart().release();
      wait_for_server.unlock();
      server_instance_cv.notify_one();

      logger::info("Torii runing!");
      server_instance->Wait();
      return 0;
    }
}
