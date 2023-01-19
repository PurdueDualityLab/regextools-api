//
// Created by charlie on 9/6/21.
//

#include "regex_repository.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

using nlohmann::json;

rereuse::db::RegexRepository::RegexRepository(const std::vector<RegexEntity>& entities) {
    for (auto &entity : entities) {
        RE2::Options re2_opts;
        re2_opts.set_log_errors(false);
        auto regex = std::make_unique<RE2>(entity.get_pattern(), re2_opts);
        if (regex->ok()) {
            this->regexes.push_back(std::move(regex));
            this->entities.push_back(entity);
        }
    }
}


std::vector<rereuse::db::RegexEntity>
rereuse::db::RegexRepository::query(const std::unique_ptr<query::BaseRegexQuery> &query) {
    std::vector<rereuse::db::RegexEntity> matching_patterns;
    for (std::size_t i = 0; i < this->regexes.size(); i++) {
        if (query->test(*this->regexes[i])) {
            matching_patterns.push_back(this->entities.at(i));
        }
    }

    return matching_patterns;
}
