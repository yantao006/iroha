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

#ifndef IROHA_ACCOUNT_HPP
#define IROHA_ACCOUNT_HPP

#include <string>
#include <cstdint>

#include "../../util/exception.hpp"

namespace object {

    class BaseObject {
        std::string str_;
        int64_t     int_;
        bool        bool_;
        float       float_;

      public:

        enum Object_type{
          STRING = 0,
          INTEGER,
          BOOLEAN,
          FLOAT,
          NONE
        } object_type;

        std::string type2str(const Object_type t) const{
          switch (t) {
            case STRING:  return "string";
            case INTEGER: return "int";
            case BOOLEAN: return "bool";
            case FLOAT:   return "float";
            case NONE:    return "none";
            default:      return "Other";
          }
        }

        explicit BaseObject(std::string s):
          str_(s),
          object_type(Object_type::STRING)
        {}

        explicit BaseObject(int i):
          int_(i),
          object_type(Object_type::INTEGER)
        {}

        explicit BaseObject(bool b):
          bool_(b),
          object_type(Object_type::BOOLEAN)
        {}

        explicit BaseObject(float f):
          float_(f),
          object_type(Object_type::FLOAT)
        {}

        Object_type type() const{
          return object_type;
        }

        operator int() const{
          if(object_type == Object_type::INTEGER){
            return int_;
          }else{
            throw exception::InvalidCastException( type2str(object_type), "int", __FILE__);
          }
        }
        operator std::string() const{
          if(object_type == Object_type::STRING){
            return str_;
          }else{
            throw exception::InvalidCastException( type2str(object_type), "string", __FILE__);
          }
        }
        operator bool() const{
          if(object_type == Object_type::BOOLEAN){
            return bool_;
          }else{
            throw exception::InvalidCastException( type2str(object_type), "bool", __FILE__);
          }
        }
        operator float() const{
          if(object_type == Object_type::FLOAT){
            return float_;
          }else{
            throw exception::InvalidCastException( type2str(object_type), "float", __FILE__);
          }
        }
    };

};  // namespace asset

#endif //IROHA_BASE_OBJECT_HPP
