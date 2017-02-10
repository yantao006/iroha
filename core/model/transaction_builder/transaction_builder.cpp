#include "transaction_builder.hpp"
#include "../../util/exception.hpp"

template<class BuildObjectT> using Builder = transaction::TransactionBuilder<BuildObjectT>;

/*****************************************************************************
  object::Domain
 *****************************************************************************/
  /*
using DomainBuilder = Builder<object::Domain>;

template<>
DomainBuilder& DomainBuilder::setOwnerPublicKey(std::string ownerPublicKey) {
  _ownerPublicKey = ownerPublicKey;
  return *this;
}

Builder<object::Domain>& Builder::setDomainName<object::Domain>(std::string name) {
  _name = name;
  return *this;
}

Builder<object::Domain>& Builder::setTrustScore<object::Domain>(double trustScore) {
  _trustScore = trustScore;
  return *this;
}

object::Domain Builder::build<object::Domain>() {

  const auto unsetMembers = enumerateUnsetMembers();
  if (not unsetMembers.empty()) {
    throw exception::transaction::UnsetBuildMembersException("object::Domain", unsetMembers);
  }

  return object::Domain(ip, publicKey, trustScore);
}

std::vector<std::string> Builder::enumerateUnsetMembers<object::Domain>();

*/