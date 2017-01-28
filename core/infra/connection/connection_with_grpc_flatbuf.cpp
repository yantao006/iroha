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
#include "../../consensus/consensus_event.hpp"
#include "../../util/logger.hpp"
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

    template<typename T>
    Offset<T>
    makeOffset(T ){}

//  This is asset
    template<>
    BaseObject
    makeOffset(flatbuffers::FlatBufferBuilder builder, BaseObject* obj){
      ObjectBuilder ob(builder);
      ob.add_text( text_ );
      ob.add_integer(object_->integer());
      ob.add_boolean(object_->boolean());
      ob.add_name( name_ );
      return ob.Finish();
    }
    template<>
    Offset<SimpleAsset>
    makeOffset(flatbuffers::FlatBufferBuilder builder, SimpleAsset* simpleAsset){
      auto name   = builder.CreateString(simpleAsset->name());
      auto object = makeOffset<BaseObject>( builder, simpleAsset->object());
      auto domain = makeOffset<Domain>( builder, simpleAsset->domain());
      auto smartContractName = builder.CreateString(simpleAsset->smartContractName());

      SimpleAsset sb(builder);
      sb.add_name( name );
      sb.add_domain( domain );
      sb.add_object( object );
      sb.add_smartContractName( smartContractName );

      return sb.Finish();
    }
    template<>
    Offset<Asset>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Asset* asset){
      auto name   = builder.CreateString( asset->name() );
      auto domain = makeOffset<Domain>( builder, asset->domain() );
      auto smartContractName = builder.CreateString( asset->smartContractName() );

      std::vector<Offset<BaseObject>> object_;
      for(int i = 0; i < asset->objects()->size(); i++){
        auto obj = makeOffset<BaseObject>( builder, asset->objects()->Get(i);
        object_.push_back( obj.Finish() );
      }
      auto objects = builder.CreateVector( object_.data(), object_.size() );

      Asset ab(builder);
      ab.add_name( name );
      ab.add_domain( domain );
      ab.add_object( objects );
      ab.add_smartContractName( smartContractName );

      return ab.Finish();
    }
    template<>
    Offset<Domain>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Domain* domain){
      auto name   = builder.CreateString( domain->name() );
      auto ownerPublicKey = builder.CreateString( domain->ownerPublicKey() );
      Domain db(builder);

      db.add_name( name );
      db.add_ownerPublicKey( ownerPublicKey );

      return db.Finish();
    }
    template<>
    Offset<Account>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Account* account){
      auto publicKey      = builder.CreateString( account->publicKey() );

      std::vector<Offset<Asset>> assets_;
      for(int i = 0; i < account->assets()->size(); i++){
        auto asset = makeOffset<Asset>( builder, account->assets()->Get(i);
        assets_.push_back( asset.Finish() );
      }
      auto assets = builder.CreateVector( assets_.data(), assets_.size() );

      Account ab(builder);
      ab.add_publicKey( publicKey );
      ab.add_assets( assets );
      return ab.Finish();
    }

    template<>
    Offset<Peer>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Peer* peer){
      auto publicKey = builder.CreateString( peer->publicKey() );
      auto address   = builder.CreateString( peer->address() );

      Account pb(builder);
      pb.add_publicKey( publicKey );
      pb.add_assets( address );
      return pb.Finish();
    }

    // This is command
    template<typename T>
    std::tuple<flatbuffers::Offset<void>,Object> makeObject(T* hasObj){
      flatbuffers::Offset<void> object;
      Object object_type;
      if( add->object_type() == Object::Object_SimpleAsset ){
        return std::make_tuple(
          makeOffset<SimpleAsset>(reinterpret_cast<const SimpleAsset *>(hasObj->object())),
          Object::Object_SimpleAsset
        );

      }else if( add->object_type() == Object::Object_Asset ){
        return std::make_tuple(
          makeOffset<Asset>(reinterpret_cast<const Asset *>(hasObj->object())),
          Object::Object_Asset
        );

      }else if( add->object_type() == Object::Object_Domain ){
        return std::make_tuple(
          makeOffset<Domain>(reinterpret_cast<const Domain *>(hasObj->object())),
          Object::Object_Domain
        );

      }else if( add->object_type() == Object::Object_Account ){
        return std::make_tuple(
          makeOffset<Account>(reinterpret_cast<const Account *>(hasObj->object())),
          Object::Object_Account
        );

      }else if( add->object_type() == Object::Object_Peer ){
        return std::make_tuple(
          makeOffset<Peer>(reinterpret_cast<const Peer *>(hasObj->object())),
          Object::Object_Peer
        );
      }
    }
    template<>
    Offset<Add>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Add* hasObj){
      auto tuple_obj = makeObject(hasObj);
      AddBuilder ab(builder);
      ab.add_object( std::get<0>(tuple_obj) );
      ab.add_object_type( std::get<1>(tuple_obj) );
      return pb.Finish();
    }
    template<>
    Offset<Transfer>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Transfer* hasObj){
      auto receiver = builder.CreateString( hasObj->receiver() );
      auto tuple_obj = makeObject(hasObj);
      TransferBuilder b(builder);
      b.add_receiver( receiver );
      b.add_object( std::get<0>(tuple_obj) );
      b.add_object_type( std::get<1>(tuple_obj) );
      return b.Finish();
    }
    template<>
    Offset<Update>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Update* hasObj){
      auto tuple_obj = makeObject(hasObj);
      UpdagteBuilder b(builder);
      b.add_object( std::get<0>(tuple_obj) );
      b.add_object_type( std::get<1>(tuple_obj) );
      return b.Finish();
    }
    template<>
    Offset<Remove>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Remove* hasObj){
      auto tuple_obj = makeObject(hasObj);
      UpdagteBuilder b(builder);
      b.add_object( std::get<0>(tuple_obj) );
      b.add_object_type( std::get<1>(tuple_obj) );
      return b.Finish();
    }
    template<>
    Offset<Batch>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Batch* obj){
      auto alias = builder.CreateString( hasObj->alias() );

      std::vector<Offset<std::String>> commands_;
      for(int i = 0; i < obj->commands()->size(); i++){
         commands_.push_back( obj->commands()->objects()->Get(i)->str() );
      }
      auto commands = builder.CreateVectorOfStrings(commands_);

      BatchBuilder b(builder);
      b.add_alias( alias );
      b.add_commands( commands );
      return b.Finish();
    }
    template<>
    Offset<Unbatch>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Unbatch* obj){
      auto alias = builder.CreateString( obj->alias() );
      UnbatchBuilder b(builder);
      b.add_alias( alias );s
      return b.Finish();
    }
    template<>
    Offset<Contract>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Contract* hasObj){
      auto command = builder.CreateString( hasObj->command() );
      auto contractName = builder.CreateString( hasObj->contractName() );

      auto tuple_obj = makeObject(hasObj);
      UpdagteBuilder b(builder);
      b.add_object( std::get<0>(tuple_obj) );
      b.add_object_type( std::get<1>(tuple_obj) );
      b.add_command( command );
      b.add_contractName( contractName );
      return b.Finish();
    }

    std::tuple<flatbuffers::Offset<void>,Command> makeCommand(Transaction* tx){
      if( tx->command_type() == Command::Command_Add ){
        return std::make_tuple(
          makeOffset<Add>(reinterpret_cast<const Add *>(tx->command())),
          Command::Command_Add
        );
      }else if( tx->command_type() == Command::Command_Transfer ){
        return std::make_tuple(
          makeOffset<Transfer>(reinterpret_cast<const Transfer *>(tx->object())),
          Object::Object_Transfer
        );
      }else if( tx->object_type() == Object::Object_Update ){
        return std::make_tuple(
          makeOffset<Update>(reinterpret_cast<const Update *>(tx->object())),
          Object::Object_Update
        );
      }else if( tx->object_type() == Object::Object_Remove ){
        return std::make_tuple(
          makeOffset<Remove>(reinterpret_cast<const Remove *>(tx->object())),
          Object::Object_Remove
        );
      }else if( tx->object_type() == Object::Object_Batch ){
        return std::make_tuple(
          makeOffset<Batch>(reinterpret_cast<const Batch *>(tx->object())),
          Object::Object_Batch
        );
      }else if( tx->object_type() == Object::Object_Unbatch ){
        return std::make_tuple(
          makeOffset<Unbatch>(reinterpret_cast<const Unbatch *>(tx->object())),
          Object::Object_Unbatch
        );
      }
    }


    // This is
    template<>
    Offset<TxSignature>
    makeOffset(flatbuffers::FlatBufferBuilder builder, TxSignature* obj){
      auto publicKey = builder.CreateString( obj->publicKey() );
      auto signature = builder.CreateString( obj->signature() );

      TxSignatureBuilder b(builder);
      b.add_publicKey( publicKey );
      b.add_signature( signature );
      return b.Finish();
    }
    template<>
    Offset<Transaction>
    makeOffset(flatbuffers::FlatBufferBuilder builder, Transaction* obj){
      auto sender = builder.CreateString( obj->sender() );

      auto command = makeCommand(obj);

      std::vector<Offset<TxSignature>> txSigs_;
      for(int i = 0; i < obj->txSignatures()->size(); i++){
        auto obj = makeOffset<TxSignature>( builder, obj->txSignatures()->Get(i);
        txSigs_.push_back( obj.Finish() );
      }
      auto txSigs = builder.CreateVector( txSigs_.data(), txSigs_.size() );
      auto hash = builder.CreateString( obj->hash() );

      TxSignatureBuilder b(builder);
      b.add_sender( sender );
      b.add_command( command );
      b.add_txSignatures( txSignatures );
      b.add_hash( hash );
      return b.Finish();
    }

    Offset<ConsensusEvent>
    makeOffset(flatbuffers::FlatBufferBuilder builder, ConsensusEvent* obj){
      auto sender = builder.CreateString( obj->sender() );

      auto command = makeCommand(obj);

      std::vector<Offset<EventSignature>> exSigs_;
      for(int i = 0; i < obj->txSignatures()->size(); i++){
        auto obj = makeOffset<EventSignature>( builder, obj->eventSignatures()->Get(i);
        exSigs_.push_back( obj.Finish() );
      }
      auto eventSignatures = builder.CreateVector( exSigs_.data(), exSigs_.size() );

      std::vector<Offset<Transaction>> txs_;
      for(int i = 0; i < obj->transaction()->size(); i++){
        auto obj = makeOffset<Transaction>( builder, obj->transaction();
        txs_.push_back( obj.Finish() );
      }
      auto transactions = builder.CreateVector( txs_.data(), txs_.size() );

      ConsensusEventBuilder b(builder);
      b.add_transaction( transactions );
      b.add_eventSignatures( eventSignatures );
      b.add_state( State::State_Commited );
      return b.Finish();
    }

*/

    class IrohaServiceImpl final : public Sumeragi::Service {
        flatbuffers::FlatBufferBuilder fbb_;
    public:
        virtual ::grpc::Status Torii(
          ::grpc::ServerContext* context,
          const flatbuffers::BufferRef<ConsensusEvent> *consensusEvent_,
          flatbuffers::BufferRef<Response> *response
        ) override {

          auto consensusEvent = GetConsensusEvent( consensusEvent_ );

          fbb_.Clear();
          auto stat_offset = CreateResponse(fbb_,
              200,
            	fbb_.CreateString("Hello, " +
                    	std::to_string(request->GetRoot()->state())
            	)
          );
          fbb_.Finish(stat_offset);
          *response = flatbuffers::BufferRef<Response>(fbb_.GetBufferPointer(),
                                                   fbb_.GetSize());
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
        const event::ConsensusEvent& event
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
