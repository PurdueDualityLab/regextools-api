//
// Created by charlie on 1/17/23.
//

#include "regex_entity.h"

#include <utility>

namespace rereuse::db {

RegexEntity::RegexEntity(std::string pattern,
                         std::string repo_location,
                         std::string file_path,
                         size_t line_number,
                         std::string license)
: pattern(std::move(pattern))
, repo_location(std::move(repo_location))
, file_path(std::move(file_path))
, line_number(line_number)
, license(std::move(license)) {}

} // db