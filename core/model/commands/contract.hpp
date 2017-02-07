/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
http://soramitsu.co.jp

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
#ifndef CORE_DOMAIN_COMMANDS_CONTRACT_HPP_
#define CORE_DOMAIN_COMMANDS_CONTRACT_HPP_

#include "../objects/object.hpp"
#include <string>

namespace command {

struct Contract {
    object::Object  object;
    std::string     name;

    Contract(
        object::Object o,
        std::string name
    ):
        object(o),
        name(name)
    {}

};

};  // namespace command

#endif  // CORE_DOMAIN_TRANSACTIONS_CONTRACT_HPP_
