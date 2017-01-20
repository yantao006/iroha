


#ifndef __IROHA_CORE_FLATBUF_FACTORY_SERVICE_HPP__
#define __IROHA_CORE_FLATBUF_FACTORY_SERVICE_HPP__
#include "../flatbuf/api_generated.h"

namespace flatbuffer {

    namespace offset {
        template<typename... Args>
        T factory(Args&&... args){
            flatbuffers::FlatBufferBuilder &_fbb,
            int w = 100;
            return T( w, std::forward<Args>(args)...);
        }

        template<>
        flatbuffers::Offset<EventSignature> factory(
          const std::string& publicKey,
          const std::string& signature
        ){
          flatbuffers::FlatBufferBuilder &builder;
          return CreateEventSignature(
            builder,
            builder.CreateString(publicKey),
            builder.CreateString(signature)
          )
        }

    }
};

#endif
