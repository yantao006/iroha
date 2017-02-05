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


#include "../../model/objects/base_object.hpp"

#include "../../consensus/connection/connection.hpp"
#include "../../consensus/consensus_event.hpp"
#include "../../util/logger.hpp"
#include "../../util/exception.hpp"
#include "../../service/peer_service.hpp"

#include <thread>

#include <grpc++/grpc++.h>

#include "../../../flatbuf/api_generated.h"
#include "../../../flatbuf/api.grpc.fb.h"

namespace connection {
    using namespace iroha;

    template<typename T>
    using Offset = flatbuffers::Offset<T>;

    std::vector<std::string> receiver_ips;
    std::vector<
        std::function<void(
            const std::string& from,
            event::ConsensusEvent& message
        )>
    > receivers;

    auto rehasher = flatbuffers::rehasher_function_t(
      [](void *pointer) -> flatbuffers::hash_value_t {
          (void)pointer;
          return 0;
    });


    class IrohaServiceImpl final : public Sumeragi::Service {
        flatbuffers::FlatBufferBuilder fbb_;
    public:

      ::grpc::Status Torii(::grpc::ClientContext* context, const flatbuffers::BufferRef<Request>& request, flatbuffers::BufferRef<Response>* response){

          // ToDo convertor

          fbb_.Clear();
          auto stat_offset = CreateResponse(fbb_,
              200,
              fbb_.CreateString("Hello, Ok")
          );
          fbb_.Finish(stat_offset);
          *response = flatbuffers::BufferRef<Response>(fbb_.GetBufferPointer(),
                                                   fbb_.GetSize());
          return grpc::Status::OK;
      }

      ::grpc::Status Verify(::grpc::ClientContext* context, const flatbuffers::BufferRef<ConsensusEvent>& request, flatbuffers::BufferRef<Response>* response){

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


    template<typename In,typename Out>
    std::unique_ptr<Out> encodeFlatbufferT(In input){}


    std::unique_ptr<iroha::BaseObjectT> encodeFlatbufferT(const object::BaseObject& obj){
      std::unique_ptr<iroha::BaseObjectT> res;
      if(obj.object_type == object::BaseObject::Object_type::STRING){
        res->text     = (std::string)obj;
      }else if(obj.object_type == object::BaseObject::Object_type::INTEGER){
        res->integer  = (int)obj;
      }else if(obj.object_type == object::BaseObject::Object_type::BOOLEAN){
        res->boolean  = (bool)obj;
      }else if(obj.object_type == object::BaseObject::Object_type::FLOAT){
        res->decimal  = (float)obj;
      }else{
        throw exception::NotImplementedException(
          "This base object type is not implemented.",__FILE__
        );
      }
      return std::move(res);
    }


    template<typename In,typename Out>
    Out encodeFlatbufferUnionT(In input){}

    ObjectUnion encodeFlatbufferUnionT(const object::Object& obj){
      ObjectUnion res;
      if(obj.type == object::ObjectValueT::asset){
        auto assetT = AssetT();
        assetT.domain   = obj.asset->domain;
        assetT.name     = obj.asset->name;

        std::vector<std::unique_ptr<BaseObjectT>> objects;
        for(const auto o: obj.asset->value){
          auto baseObj = encodeFlatbufferT(o.second);
          baseObj->name = o.first;
          objects.push_back(std::move(baseObj));
        }
        assetT.objects = std::move(objects);

        res.Set(std::move(assetT));
      }else if(obj.type == object::ObjectValueT::domain){
        auto domainT = DomainT();
        res.Set(std::move(domainT));
      }else if(obj.type == object::ObjectValueT::account){
        auto accountT = AccountT();
        res.Set(std::move(accountT));
      }else if(obj.type == object::ObjectValueT::peer){
        auto peerT = PeerT();
        res.Set(std::move(peerT));
      }else{
        throw exception::NotImplementedException(
          "This object is not implemented.",__FILE__
        );
      }
      return res;
    }

    CommandUnion encodeFlatbufferUnionT(const std::unique_ptr<command::Command>& command){
      CommandUnion res;
      if(command->getCommandName() == command::Command::CommandType::ADD){
        auto addT = AddT();
        addT.object = encodeFlatbufferUnionT(command->getObject());
        res.Set(std::move(addT));
      }else if(command->getCommandName() == command::Command::CommandType::TRANSFER){
        auto transferT = TransferT();
        transferT.object = encodeFlatbufferUnionT(command->getObject());
        res.Set(std::move(transferT));
      }else if(command->getCommandName() == command::Command::CommandType::UPDATE){
        auto updateT = UpdateT();
        updateT.object = encodeFlatbufferUnionT(command->getObject());
        res.Set(std::move(updateT));
      }else if(command->getCommandName() == command::Command::CommandType::REMOVE){
        auto removeT = RemoveT();
        removeT.object = encodeFlatbufferUnionT(command->getObject());
        res.Set(std::move(removeT));
      }else if(command->getCommandName() == command::Command::CommandType::CONTRACT){
        auto contractT = ContractT();
        contractT.object = encodeFlatbufferUnionT(command->getObject());
        res.Set(std::move(contractT));
      }else if(command->getCommandName() == command::Command::CommandType::BATCH){
        auto batchT = BatchT();
        res.Set(std::move(batchT));
      }else if(command->getCommandName() == command::Command::CommandType::UNBATCH){
        auto unbatchT = UnbatchT();
        res.Set(std::move(unbatchT));
      }else{
        throw exception::NotImplementedException(
          "This command is not implemented.",__FILE__
        );
      }
      return res;
    }

    std::string text;
    int32_t integer;
    bool boolean;
    float decimal;

    std::unique_ptr<iroha::TransactionT> encodeFlatbufferT(const event::Transaction& tx){
      std::unique_ptr<iroha::TransactionT> res;
      res->sender = tx.senderPublicKey;
      res->hash   = tx.hash;
      std::vector<std::unique_ptr<TxSignatureT>> tsTv;
      for(auto&& t: tx.txSignatures()){
        std::unique_ptr<TxSignatureT> ts;
        ts->publicKey = t.publicKey;
        ts->signature = t.signature;
        tsTv.push_back( std::move(ts) );
      }
      res->command = encodeFlatbufferUnionT(tx.command);
      res->txSignatures = std::move(tsTv);
      return std::move(res);
    }

    std::unique_ptr<iroha::ConsensusEventT> encodeFlatbufferT(const event::ConsensusEvent& event){
        std::unique_ptr<iroha::ConsensusEventT> res;
        std::vector<std::unique_ptr<TransactionT>> txTv;
        for(auto&& tx: event.transactions){
          txTv.push_back(std::move(encodeFlatbufferT(tx)));
        }
        std::vector<std::unique_ptr<EventSignatureT>> esTv;
        for(auto&& e: event.eventSignatures()){
          std::unique_ptr<EventSignatureT> es;
          es->publicKey = e.publicKey;
          es->signature = e.signature;
          esTv.push_back( std::move(es) );
        }
        res->transaction = std::move(txTv);
        res->eventSignatures = std::move(esTv);
        res->state = State::State_Undetermined;
        return std::move(res);
    }

    bool send(
        const std::string& ip,
        const event::ConsensusEvent& event
    ) {
        logger::info("connection") << "start send";
        if (find(receiver_ips.begin(), receiver_ips.end(), ip) != receiver_ips.end()){

            logger::info("connection")  <<  "create client";
            auto channel = grpc::CreateChannel(
              "localhost:50051",
              grpc::InsecureChannelCredentials()
            );
            auto stub = Sumeragi::NewStub(channel);
            grpc::ClientContext context;

            iroha::ConsensusEventT eventT;
/*
            struct ConsensusEventT : public flatbuffers::NativeTable {
              typedef ConsensusEvent TableType;
              std::vector<std::unique_ptr<TransactionT>> transaction;
              std::vector<std::unique_ptr<EventSignatureT>> eventSignatures;
              State state;
              ConsensusEventT()
                  : state(State_Undetermined) {
              }
*/
//            ConsensusEventT *UnPack(const flatbuffers::resolver_function_t *_resolver = nullptr) const;
//            void UnPackTo(ConsensusEventT *_o, const flatbuffers::resolver_function_t *_resolver = nullptr) const;
//            static flatbuffers::Offset<ConsensusEvent> Pack(flatbuffers::FlatBufferBuilder &_fbb, const ConsensusEventT* _o, const flatbuffers::rehasher_function_t *_rehasher = nullptr);          flatbuffers::FlatBufferBuilder fbb;
/*
            flatbuffers::FlatBufferBuilder fbb;
            fbb.Finish(iroha::ConsensusEvent::Pack( fbb, &event));
            auto request = flatbuffers::BufferRef<iroha::ConsensusEvent>(
              fbb.GetBufferPointer(),
              fbb.GetSize()
            );
            flatbuffers::BufferRef<Response> response;

            auto status = stub->Verify(&context, request, &response);

            if (status.ok()) {
              auto message = response.GetRoot()->message();
              logger::info("connection sumeragi's response: ") << message->str();
            } else {
              logger::error("connection sumeragi response error");
            }
*/
            return true;
        }else{
            logger::error("connection") <<  "not found";
            return false;
        }
    }

    bool sendAll(
        const event::ConsensusEvent& event
    ) {
        for (auto& ip : receiver_ips){
            if (ip != peer::getMyIp()){
                send( ip, event);
            }
        }
        return true;
    }

    bool receive(
      const std::function<void(
        const std::string&,
        event::ConsensusEvent&
      )>& callback) {
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
