//
// Created by charlie on 1/25/23.
//

#include "regex_entity.h"

#include <utility>

RegexEntity::RegexEntity(const std::string& regex, std::vector<std::string> strings)
: strings(std::move(strings)) {
    re2::RE2 re2_regex(regex);
    if (re2_regex.ok()) {
        this->regex = std::make_unique<re2::RE2>(regex);
    } else {
        throw std::runtime_error("Invalid regex");
    }
}
