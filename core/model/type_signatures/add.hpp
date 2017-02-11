#ifndef CORE_MODEL_TYPE_SIGNATURES_ADD_HPP
#define CORE_MODEL_TYPE_SIGNATURES_ADD_HPP

#include "tags.hpp"

namespace type_signatures {

namespace detail {
template <class T, class... Rest>
struct head {
  using type = T;
};
}

template <class... Ts>
struct Add {
  using type = typename detail::head< Ts... >::type;
  object::Object object;
  Add(type o): object(o) {}
};

template <>
struct Add<type_signatures::Add<object::Asset, To<object::Account>>> {
  object::Object object;
  Add(object::Account rhs): object(rhs) {}
};

}



#endif