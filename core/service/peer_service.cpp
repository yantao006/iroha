/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language gover
ning permissions and
limitations under the License.
*/

//
// Created by Takumi Yamashita on 2017/03/16.
//

#include <deque>
#include <regex>
#include <algorithm>
#include <json.hpp>

#include <service/peer_service.hpp>

#include <crypto/base64.hpp>
#include <util/logger.hpp>
#include <util/exception.hpp>
#include <consensus/connection/connection.hpp>

#include <transaction_builder/transaction_builder.hpp>
#include <repository/transaction_repository.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include<service/peer_service.hpp>

#include <infra/config/config_format.hpp>
#include <infra/protobuf/api.pb.h>


namespace peer {
    using PeerServiceConfig = config::PeerServiceConfig;
    using txbuilder::TransactionBuilder;
    using type_signatures::Update;
    using type_signatures::Add;
    using type_signatures::Remove;
    using type_signatures::Peer;
    using nlohmann::json;

    std::vector<peer::Node> peerList;
    bool is_active;


    namespace myself {
        std::string getPublicKey(){
            return PeerServiceConfig::getInstance().getMyPublicKeyWithDefault("Sht5opDIxbyK+oNuEnXUs5rLbrvVgb2GjSPfqIYGFdU=");
        }
        std::string getPrivateKey(){
            return PeerServiceConfig::getInstance().getMyPrivateKeyWithDefault("aGIuSZRhnGfFyeoKNm/NbTylnAvRfMu3KumOEfyT2HPf36jSF22m2JXWrdCmKiDoshVqjFtZPX3WXaNuo9L8WA==");
        }
        std::string getIp(){
            return PeerServiceConfig::getInstance().getMyIpWithDefault("172.17.0.6");
        }
        bool isActive(){
            return is_active;
        }
        void activate(){
            is_active = true;
        }
        void stop(){
            is_active = false;
        }
        // equatl to isSumeragi
        bool isLeader(){
            auto sorted_peers = service::getPeerList();
            if( sorted_peers.empty() ) return false;
            return (*sorted_peers.begin())->getPublicKey() == getPublicKey() &&
                   (*sorted_peers.begin())->getIP() == getIp();
        }
    }

    namespace service {

        // this function must be invoke before use peer-service.
        void initialize(){
            if (!peerList.empty()) return;
            for (const auto& peer : PeerServiceConfig::getInstance().getGroup() ) {
                peerList.emplace_back( peer["ip"].get<std::string>(),
                                       peer["publicKey"].get<std::string>(),
                                       PeerServiceConfig::getInstance().getMaxTrustScore() );
            }
        }

        size_t getMaxFaulty(){
            return std::max( 0, ((int)getPeerList().size()-1) / 3 );
        }
        std::vector<std::unique_ptr<peer::Node>> getPeerList() {
            initialize();

            std::vector<std::unique_ptr<peer::Node>> nodes;
            for(const auto &node : peerList) {
                if(node.isOK()) {
                    nodes.push_back(std::make_unique<peer::Node>(node.getIP(),
                                                                 node.getPublicKey(),
                                                                 node.getTrustScore()));
                }
            }

            sort(nodes.begin(), nodes.end(), [](const auto &a, const auto &b) {
                return a->getTrustScore() > b->getTrustScore();
            } );
            logger::debug("getPeerList") << std::to_string(nodes.size());
            for(const auto &node : nodes ) {
                logger::debug("getPeerList") << node->getIP() + " " <<node->getPublicKey();
            }
            return nodes;
        }
        std::vector<std::string> getIpList(){
            std::vector<std::string> ret_ips;
            for(const auto &node : getPeerList() )
                ret_ips.push_back( node->getIP() );
            return ret_ips;
        }
        // is exist which peer?
        bool isExistIP(const std::string &ip){
            return findPeerIP( std::move(ip) ) != peerList.end();
        }
        bool isExistPublicKey(const std::string &publicKey){
            return findPeerPublicKey( std::move(publicKey) ) != peerList.end();
        }
        std::vector<peer::Node>::iterator findPeerIP(const std::string &ip){
            initialize();
            return std::find_if( peerList.begin(), peerList.end(),
                                 [&ip]( const peer::Node& p ) { return p.getIP() == ip; } );
        }
        std::vector<peer::Node>::iterator findPeerPublicKey(const std::string &publicKey){
            initialize();
            return std::find_if( peerList.begin(), peerList.end(),
                                 [&publicKey]( const peer::Node& p ) { return p.getPublicKey() == publicKey; } );
        }
        std::unique_ptr<peer::Node> leaderPeer(){
            return std::move( *getPeerList().begin() );
        }
    }

    namespace transaction {

        // Initialize
        namespace izanami {
            void finished(){
                std::string leader_ip = service::leaderPeer()->getIP();
                auto txPeer = TransactionBuilder<Update<Peer>>()
                        .setSenderPublicKey(myself::getPublicKey())
                        .setPeer(txbuilder::createPeer( myself::getPublicKey(), myself::getIp(), txbuilder::createTrust(0.0, true)))
                        .build();
                connection::iroha::PeerService::Sumeragi::send( leader_ip, txPeer );
                myself::activate();
            }
            //invoke next to addPeer
            bool started(const Node& peer){
                logger::debug("peer-service") << "in sendAllTransactionToNewPeer";
                // when my node is not active, it don't send data.
                if( !(service::findPeerPublicKey( myself::getPublicKey() )->isOK()) ) return false;

                uint64_t code = 0UL;
                {   // Send PeerList data ( Reason: Can't do to construct peerList for only transaction infomation. )
                    logger::debug("peer-service") << "send all peer infomation";
                    auto sorted_peerList = service::getPeerList();
                    auto txResponse = Api::TransactionResponse();
                    txResponse.set_message( "Initilize send now Active PeerList info" );
                    txResponse.set_code( code++ );
                    for (auto &&peer : sorted_peerList) {
                        auto txPeer = TransactionBuilder<Add<Peer>>()
                                .setSenderPublicKey(myself::getPublicKey())
                                .setPeer(txbuilder::createPeer(peer->getPublicKey(), peer->getIP(),
                                                               txbuilder::createTrust(PeerServiceConfig::getInstance().getMaxTrustScore(), true)))
                                .build();
                        txResponse.add_transaction()->CopyFrom(txPeer);
                    }
                    if( !connection::iroha::PeerService::Izanami::send( peer.getIP(), txResponse ) ) return false;
                }

                if(0){   // WIP(leveldb don't active) Send transaction data separated block to new peer.
                    logger::debug("peer-service") << "send all transaction infomation";
                    auto transactions = repository::transaction::findAll();
                    int block_size = 500;
                    for (int i = 0; i < transactions.size(); i += block_size) {
                        auto txResponse = Api::TransactionResponse();
                        txResponse.set_message("Midstream send Transactions");
                        txResponse.set_code(code++);
                        for (int j = i; j < i + block_size; j++) {
                            txResponse.add_transaction()->CopyFrom(transactions[j]);
                        }
                        if (!connection::iroha::PeerService::Izanami::send(peer.getIP(), txResponse)) return false;
                    }
                }

                {   // end-point
                    logger::debug("peer-service") << "send end-point";
                    auto txResponse = Api::TransactionResponse();
                    txResponse.set_message("Finished send Transactions");
                    txResponse.set_code(code++);
                    if (!connection::iroha::PeerService::Izanami::send(peer.getIP(), txResponse)) return false;
                }
                return true;
            }
        }


        namespace isssue {
            // invoke to issue transaction
            void add(const peer::Node &peer){
                if( service::isExistIP(peer.getIP()) || service::isExistPublicKey(peer.getPublicKey()) ) return;
                auto txPeer = TransactionBuilder<Add<Peer>>()
                        .setSenderPublicKey(myself::getPublicKey())
                        .setPeer( txbuilder::createPeer( peer.getPublicKey(), peer.getIP(), txbuilder::createTrust(PeerServiceConfig::getInstance().getMaxTrustScore(),false ) ) )
                        .build();
                connection::iroha::PeerService::Sumeragi::send( myself::getPublicKey(), txPeer );
            }
            void distruct(const std::string &publicKey){
                if( !service::isExistPublicKey(publicKey) ) return;
                auto txPeer = TransactionBuilder<Update<Peer>>()
                        .setSenderPublicKey(myself::getPublicKey())
                        .setPeer(txbuilder::createPeer(publicKey, peer::Node::defaultIP(), txbuilder::createTrust(-1.0, true)))
                        .build();
                connection::iroha::PeerService::Sumeragi::send( myself::getPublicKey(), txPeer );
            }
            void remove(const std::string &publicKey){
                if( !service::isExistPublicKey(publicKey) ) return;
                auto txPeer = TransactionBuilder<Remove<Peer>>()
                        .setSenderPublicKey(myself::getPublicKey())
                        .setPeer(txbuilder::createPeer(publicKey, peer::Node::defaultIP(), txbuilder::createTrust(-PeerServiceConfig::getInstance().getMaxTrustScore(), false)))
                        .build();
                connection::iroha::PeerService::Sumeragi::send( myself::getPublicKey(), txPeer );
            }
            void credit(const std::string &publicKey){
                if( !service::isExistPublicKey(publicKey) ) return;
                if( service::findPeerPublicKey(publicKey)->getTrustScore() == PeerServiceConfig::getInstance().getMaxTrustScore() ) return;
                auto txPeer = TransactionBuilder<Update<Peer>>()
                        .setSenderPublicKey(myself::getPublicKey())
                        .setPeer(txbuilder::createPeer(publicKey, peer::Node::defaultIP(),
                                                       txbuilder::createTrust( +1.0, true)))
                        .build();
                connection::iroha::PeerService::Sumeragi::send( myself::getPublicKey(), txPeer );
            }
        }
        namespace executor {
            // invoke when execute transaction
            bool add(const peer::Node &peer){
                try {
                    if( service::isExistIP( peer.getIP() ) )
                        throw exception::service::DuplicationIPException(peer.getIP());
                    if( service::isExistPublicKey( peer.getPublicKey() ) )
                        throw exception::service::DuplicationPublicKeyException(peer.getPublicKey());
                    peerList.emplace_back( std::move(peer));
                } catch( exception::service::DuplicationPublicKeyException& e ) {
                    logger::warning("addPeer") << e.what();
                    return false;
                } catch( exception::service::DuplicationIPException& e ) {
                    logger::warning("addPeer") << e.what();
                    return false;
                }
                return true;
            }
            bool remove(const std::string &publicKey){
                try {
                    auto it = service::findPeerPublicKey( publicKey );
                    if ( !service::isExistPublicKey( publicKey ) )
                        throw exception::service::UnExistFindPeerException(publicKey);
                    peerList.erase(it);
                } catch (exception::service::UnExistFindPeerException& e) {
                    logger::warning("removePeer") << e.what();
                    return false;
                }
                return true;
            }
            bool update(const std::string &publicKey,
                        const peer::Node &peer){
                try {
                    auto it = service::findPeerPublicKey( publicKey );
                    if (it == peerList.end() )
                        throw exception::service::UnExistFindPeerException( publicKey );

                    if ( !peer.isDefaultPublicKey() ) {
                        auto upd_it = service::findPeerPublicKey(peer.getPublicKey());
                        if( upd_it != it && upd_it != peerList.end() ) throw exception::service::DuplicationPublicKeyException(peer.getPublicKey());
                        it->setPublicKey( peer.getPublicKey() );
                    }

                    if ( !peer.isDefaultIP() ) {
                        auto upd_it = service::findPeerIP(peer.getIP());
                        if( upd_it != it && upd_it != peerList.end() ) throw exception::service::DuplicationIPException(peer.getIP());
                        it->setIP( peer.getIP() );
                    }

                    if ( it->getTrustScore() != 0.0 ) {
                        it->setTrustScore( std::min( PeerServiceConfig::getInstance().getMaxTrustScore(), it->getTrustScore()+peer.getTrustScore()) );
                    }

                    if( it->isOK() != peer.isOK() ) {
                        it->setOK( peer.isOK() );
                    }

                } catch ( exception::service::UnExistFindPeerException& e ) {
                    logger::warning("updatePeer") << e.what();
                    return false;
                } catch( exception::service::DuplicationPublicKeyException& e ) {
                    logger::warning("updatePeer") << e.what();
                    return false;
                } catch ( exception::service::DuplicationIPException& e ) {
                    logger::warning("updatePeer") << e.what();
                    return false;
                }
                return true;
            }
        }
        namespace validator {
            // invoke when validator transaction
            bool add(const peer::Node &peer){
                try {
                    if( service::isExistIP( peer.getIP() ) )
                        throw exception::service::DuplicationIPException(std::move(peer.getIP()));
                    if( service::isExistPublicKey( peer.getPublicKey() ) )
                        throw exception::service::DuplicationPublicKeyException(std::move(peer.getPublicKey()));
                } catch( exception::service::DuplicationPublicKeyException& e ) {
                    logger::warning("validate addPeer") << e.what();
                    return false;
                } catch( exception::service::DuplicationIPException& e ) {
                    logger::warning("validate addPeer") << e.what();
                    return false;
                }
                return true;
            }
            bool remove(const std::string &publicKey){
                try {
                    if ( !service::isExistPublicKey( publicKey ) )
                        throw exception::service::UnExistFindPeerException(publicKey);
                } catch (exception::service::UnExistFindPeerException& e) {
                    logger::warning("validate removePeer") << e.what();
                    return false;
                }
                return true;
            }
            bool update(const std::string &publicKey,
                        const peer::Node &peer){
                try {
                    auto it = service::findPeerPublicKey( publicKey );
                    if (it == peerList.end() )
                        throw exception::service::UnExistFindPeerException( publicKey );

                    if ( !peer.isDefaultPublicKey() ) {
                        auto upd_it = service::findPeerPublicKey(peer.getPublicKey());
                        if( upd_it != it && upd_it != peerList.end() ) throw exception::service::DuplicationPublicKeyException(peer.getPublicKey());
                    }

                    if ( !peer.isDefaultIP() ) {
                        auto upd_it = service::findPeerIP(peer.getIP());
                        if( upd_it != it && upd_it != peerList.end() ) throw exception::service::DuplicationIPException(peer.getIP());
                    }

                } catch ( exception::service::UnExistFindPeerException& e ) {
                    logger::warning("updatePeer") << e.what();
                    return false;
                } catch( exception::service::DuplicationPublicKeyException& e ) {
                    logger::warning("updatePeer") << e.what();
                } catch( exception::service::DuplicationIPException& e ) {
                    logger::warning("updatePeer") << e.what();
                    return false;
                }
                return true;
            }
        }
    }



}