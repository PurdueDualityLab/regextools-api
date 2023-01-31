//
// Created by charlie on 1/17/23.
//

#include "regex_entity.h"

#include <utility>

namespace rereuse::db {

RegexEntity::RegexEntity(std::string pattern,
                         std::string id)
: pattern(std::move(pattern))
, id(std::move(id))
{}

} // db