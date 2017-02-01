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
    makeOffset(const T* obj){}

//  This is asset
    template<>
    Offset<BaseObject>
    makeOffset(const BaseObject* object_){
      flatbuffers::FlatBufferBuilder builder;
      auto text_   = builder.CreateString(object_->text());
      auto integer_ = object_->integer();
      auto boolean_ = object_->boolean();
      auto name_ = builder.CreateString(object_->name());

      BaseObjectBuilder ob(builder);
      ob.add_text( text_ );
      ob.add_integer( integer_ );
      ob.add_boolean( boolean_ );
      ob.add_name( name_ );
      return ob.Finish();
    }

    template<>
    Offset<SimpleAsset>
    makeOffset(const SimpleAsset* simpleAsset){
      flatbuffers::FlatBufferBuilder builder;
      auto name   = builder.CreateString(simpleAsset->name());
      auto object = makeOffset<BaseObject>( simpleAsset->object());
      auto domain = makeOffset<Domain>( simpleAsset->domain());
      auto smartContractName = builder.CreateString(simpleAsset->smartContractName());

      SimpleAssetBuilder sb(builder);
      sb.add_name( name );
      sb.add_domain( domain );
      sb.add_object( object );
      sb.add_smartContractName( smartContractName );

      return sb.Finish();
    }

/*
    template<>
    Offset<Domain>
    makeOffset(const Domain* domain){
      flatbuffers::FlatBufferBuilder builder;
      auto name   = builder.CreateString( domain->name() );
      auto ownerPublicKey = builder.CreateString( domain->ownerPublicKey() );
      Domain db(builder);

      db.add_name( name );
      db.add_ownerPublicKey( ownerPublicKey );

      return db.Finish();
    }
*/

    template<>
    Offset<Asset>
    makeOffset(const Asset* asset){
      flatbuffers::FlatBufferBuilder builder;
      auto name   = builder.CreateString( asset->name() );
      auto domain = makeOffset<Domain>( asset->domain() );
      auto smartContractName = builder.CreateString( asset->smartContractName() );

      std::vector<Offset<BaseObject>> object_;
      for(int i = 0; i < asset->objects()->size(); i++){
        auto obj = makeOffset<BaseObject>( asset->objects()->Get(i));
        object_.push_back( obj );
      }
      auto objects = builder.CreateVector( object_.data(), object_.size() );

      AssetBuilder ab(builder);
      ab.add_name( name );
      ab.add_domain( domain );
      ab.add_objects( objects );
      ab.add_smartContractName( smartContractName );

      return ab.Finish();
    }

    template<>
    Offset<Account>
    makeOffset(const Account* account){
      flatbuffers::FlatBufferBuilder builder;
      auto publicKey      = builder.CreateString( account->publicKey() );

      std::vector<Offset<Asset>> assets_;
      for(int i = 0; i < account->assets()->size(); i++){
        auto asset = makeOffset<Asset>( account->assets()->Get(i));
        assets_.push_back( asset );
      }
      auto assets = builder.CreateVector( assets_.data(), assets_.size() );

      AccountBuilder ab(builder);
      ab.add_publicKey( publicKey );
      ab.add_assets( assets );
      return ab.Finish();
    }

    template<>
    Offset<Peer>
    makeOffset(const Peer* peer){
      flatbuffers::FlatBufferBuilder builder;
      auto publicKey = builder.CreateString( peer->publicKey() );
      auto address   = builder.CreateString( peer->address() );

      PeerBuilder pb(builder);
      pb.add_publicKey( publicKey );
      pb.add_address( address );
      return pb.Finish();
    }

    // This is command
    template<typename T>
    std::tuple<flatbuffers::Offset<void>,Object> makeObject(T* hasObj){
      flatbuffers::FlatBufferBuilder builder;
      if( hasObj->object_type() == Object::Object_SimpleAsset ){
        return std::make_tuple(
          makeOffset<SimpleAsset>(reinterpret_cast<const SimpleAsset *>(hasObj->object())).Union(),
          Object::Object_SimpleAsset
        );
      }else if( hasObj->object_type() == Object::Object_Asset ){
        return std::make_tuple(
          makeOffset<Asset>(reinterpret_cast<const Asset *>(hasObj->object())).Union().Union(),
          Object::Object_Asset
        );
      }else if( hasObj->object_type() == Object::Object_Domain ){
        return std::make_tuple(
          makeOffset<Domain>(reinterpret_cast<const Domain *>(hasObj->object())).Union(),
          Object::Object_Domain
        );
      }else if( hasObj->object_type() == Object::Object_Account ){
        return std::make_tuple(
          makeOffset<Account>(reinterpret_cast<const Account *>(hasObj->object())).Union(),
          Object::Object_Account
        );
      }else if( hasObj->object_type() == Object::Object_Peer ){
        return std::make_tuple(
          makeOffset<Peer>(reinterpret_cast<const Peer *>(hasObj->object())).Union(),
          Object::Object_Peer
        );
      }
    }
    template<>

    Offset<Add>
    makeOffset(const Add* hasObj){
      flatbuffers::FlatBufferBuilder builder;
      auto tuple_obj = makeObject(hasObj);
      AddBuilder ab(builder);
      ab.add_object( std::get<0>(tuple_obj) );
      ab.add_object_type( std::get<1>(tuple_obj) );
      return ab.Finish();
    }
    template<>
    Offset<Transfer>
    makeOffset(const Transfer* hasObj){
      flatbuffers::FlatBufferBuilder builder;
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
    makeOffset(const Update* hasObj){
      flatbuffers::FlatBufferBuilder builder;
      auto tuple_obj = makeObject(hasObj);
      UpdateBuilder b(builder);
      b.add_object( std::get<0>(tuple_obj) );
      b.add_object_type( std::get<1>(tuple_obj) );
      return b.Finish();
    }
    template<>
    Offset<Remove>
    makeOffset(const Remove* hasObj){
      flatbuffers::FlatBufferBuilder builder;
      auto tuple_obj = makeObject(hasObj);
      RemoveBuilder b(builder);
      b.add_object( std::get<0>(tuple_obj) );
      b.add_object_type( std::get<1>(tuple_obj) );
      return b.Finish();
    }
    template<>
    Offset<Batch>
    makeOffset(const Batch* obj){
      flatbuffers::FlatBufferBuilder builder;
      auto alias = builder.CreateString( obj->alias() );
/*
      WIP
      std::vector<std::string> commands_;
      for(int i = 0; i < obj->commands()->size(); i++){
         commands_.push_back( obj->commands()->objects()->Get(i)->str() );
      }
      auto commands = builder.CreateVectorOfStrings(commands_);
      BatchBuilder b(builder);
      b.add_alias( alias );
      b.add_commands( commands );
*/
      BatchBuilder b(builder);
      return b.Finish();
    }
    template<>
    Offset<Unbatch>
    makeOffset(const Unbatch* obj){
      flatbuffers::FlatBufferBuilder builder;
      auto alias = builder.CreateString( obj->alias() );
      UnbatchBuilder b(builder);
      b.add_alias( alias );
      return b.Finish();
    }
    template<>
    Offset<Contract>
    makeOffset(const Contract* hasObj){
      flatbuffers::FlatBufferBuilder builder;
      auto command = builder.CreateString( hasObj->command() );
      auto contractName = builder.CreateString( hasObj->contractName() );

      auto tuple_obj = makeObject(hasObj);
      ContractBuilder b(builder);
      b.add_object( std::get<0>(tuple_obj) );
      b.add_object_type( std::get<1>(tuple_obj) );
      b.add_command( command );
      b.add_contractName( contractName );
      return b.Finish();
    }

    std::tuple<flatbuffers::Offset<void>,Command> makeCommand(const Transaction* tx){
      if( tx->command_type() == Command::Command_Add ){
        return std::make_tuple(
          makeOffset<Add>(reinterpret_cast<const Add *>(tx->command())).Union(),
          tx->command_type()
        );
      }else if( tx->command_type() == Command::Command_Transfer ){
        return std::make_tuple(
          makeOffset<Transfer>(reinterpret_cast<const Transfer *>(tx->command())).Union(),
          tx->command_type()
        );
      }else if( tx->command_type() == Command::Command_Update ){
        return std::make_tuple(
          makeOffset<Update>(reinterpret_cast<const Update *>(tx->command())).Union(),
          tx->command_type()
        );
      }else if( tx->command_type() == Command::Command_Remove ){
        return std::make_tuple(
          makeOffset<Remove>(reinterpret_cast<const Remove *>(tx->command())).Union(),
          tx->command_type()
        );
      }else if( tx->command_type() == Command::Command_Batch ){
        return std::make_tuple(
          makeOffset<Batch>(reinterpret_cast<const Batch *>(tx->command())).Union(),
          tx->command_type()
        );
      }else if( tx->command_type() == Command::Command_Unbatch ){
        return std::make_tuple(
          makeOffset<Unbatch>(reinterpret_cast<const Unbatch *>(tx->command())).Union(),
          tx->command_type()
        );
      }
    }


    // This is
    template<>
    Offset<TxSignature>
    makeOffset(const TxSignature* obj){
      flatbuffers::FlatBufferBuilder builder;
      auto publicKey = builder.CreateString( obj->publicKey() );
      auto signature = builder.CreateString( obj->signature() );

      TxSignatureBuilder b(builder);
      b.add_publicKey( publicKey );
      b.add_signature( signature );
      return b.Finish();
    }
    template<>
    Offset<Transaction>
    makeOffset(const Transaction* obj){
      flatbuffers::FlatBufferBuilder builder;
      auto sender = builder.CreateString( obj->sender() );

      auto command = makeCommand(obj);

      std::vector<Offset<TxSignature>> txSigs_;
      for(int i = 0; i < obj->txSignatures()->size(); i++){
        txSigs_.push_back(  makeOffset<TxSignature>( obj->txSignatures()->Get(i)));
      }
      auto txSigs = builder.CreateVector( txSigs_.data(), txSigs_.size() );
      auto hash = builder.CreateString( obj->hash() );

      TransactionBuilder b(builder);
      b.add_sender( sender );
      b.add_command( std::get<0>(command) );
      b.add_command_type( std::get<1>(command) );
      b.add_txSignatures( txSigs );
      b.add_hash( hash );
      return b.Finish();
    }

    Offset<iroha::ConsensusEvent>
    makeOffset(const iroha::ConsensusEvent* obj){
      flatbuffers::FlatBufferBuilder builder;

      std::vector<Offset<EventSignature>> exSigs_;
      for(int i = 0; i < obj->eventSignatures()->size(); i++){
        auto esig = makeOffset<EventSignature>( obj->eventSignatures()->Get(i) );
        exSigs_.push_back( esig );
      }
      auto eventSignatures = builder.CreateVector( exSigs_.data(), exSigs_.size() );

      std::vector<Offset<Transaction>> txs_;
      for(int i = 0; i < obj->transaction()->size(); i++){
        auto tx = makeOffset<Transaction>( obj->transaction()->Get(i) );
        txs_.push_back( tx );
      }
      auto transactions = builder.CreateVector( txs_.data(), txs_.size() );

      ConsensusEventBuilder b(builder);
      b.add_transaction( transactions );
      b.add_eventSignatures( eventSignatures );
      b.add_state( State::State_Commited );
      return b.Finish();
    }

    class IrohaServiceImpl final : public Sumeragi::Service {
        flatbuffers::FlatBufferBuilder fbb_;
    public:

      ::grpc::Status Torii(::grpc::ClientContext* context, const flatbuffers::BufferRef<Request>& request, flatbuffers::BufferRef<Response>* response){

      }
      ::grpc::Status Verify(::grpc::ClientContext* context, const flatbuffers::BufferRef<ConsensusEvent>& request, flatbuffers::BufferRef<Response>* response){

      }

/*
        virtual ::grpc::Status Sumeragi::Stub::Torii(
          ::grpc::ServerContext* context,
          const flatbuffers::BufferRef<ConsensusEvent> *consensusEvent_,
          flatbuffers::BufferRef<Response> *response
        ) override {

          auto consensusEvent = GetConsensusEvent( consensusEvent_ );

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
*/
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
            auto stub = Sumeragi::NewStub(channel);
            grpc::ClientContext context;

            flatbuffers::FlatBufferBuilder fbb;
            fbb.Finish(makeOffset<ConsensusEvent>(&event));

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
