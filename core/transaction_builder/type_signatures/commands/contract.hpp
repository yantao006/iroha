#ifndef CORE_MODEL_TYPE_SIGNATURES_CONTRACT_HPP
#define CORE_MODEL_TYPE_SIGNATURES_CONTRACT_HPP

#include "tags.hpp"
#include "../objects.hpp"

namespace type_signatures {

template <class... Ts>
struct Contract {
  using type = typename detail::head< Ts... >::type;
};

}

#endif