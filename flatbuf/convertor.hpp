

#ifndef __IROHA_FLATBUF_CONVERTOR__
#define __IROHA_FLATBUF_CONVERTOR__

#include "api_generated.h"

#include "../core/consensus/consensus_event.hpp"
#include "../core/model/"

#include <vector>

namespace flatbuffers{
namespace convertor{

  template<typename T>
  T decode(const T& buf){
    throw NotImplementedException("flatbuffers::convertor::decode<"+typeid(buf).name()+">", "convertor.hpp");
  }

  template<typename T>
  std::vector<T> decodeVector(const flatbuffers::Vector<flatbuffers::Offset<T>>& array){
    throw NotImplementedException("flatbuffers::convertor::decodeVector<"+typeid(buf).name()+">", "convertor.hpp");
  }

  template<typename T>
  std::vector<event::ConsensusEvent::EventSignature>
    decodeVector(const flatbuffers::Vector<event::ConsensusEvent::EventSignature>& eSigs
  ){
    std::vector<event::ConsensusEvent::EventSignature> eventSignatures;
    for(
      auto it = eSigs->begin(); it != eSigs->end(); it++
    ) {
      eventSignatures.push_back( event::ConsensusEvent::EventSignature( it->publicKey(), it->signature() ));
    }
    return eventSignatures;
  }

  template<typename T>
  std::vector<transaction::Transaction::TxSignature> decodeVector(
    const flatbuffers::Vector<transaction::Transaction::TxSignature>& txSigs
  ){
    std::vector<transaction::Transaction::TxSignature> transactionSignatures;
    for(
      auto it = eSigs->begin(); it != eSigs->end(); it++
    ) {
      transactionSignatures.push_back( transaction::Transaction::TxSignature( it->publicKey(), it->signature() ));
    }
    return transactionSignatures;
  }

  template<typename T>
  std::vector<transaction::Transaction> decodeVector(
    const flatbuffers::Vector<transaction::Transaction>& txSigs
  ){
    std::vector<transaction::Transaction::TxSignature> transactionSignatures;
    for(
      auto it = eSigs->begin(); it != eSigs->end(); it++
    ) {
      transactionSignatures.push_back( transaction::Transaction::TxSignature( it->publicKey(), it->signature() ));
    }
    return transactionSignatures;
  }


  template<>
  decode(const ConsensusEvent& buf){
    event::ConsensusEvent event;
    buf.eventSignatures()
  }

};
};


#endif
