//
// Created by charlie on 9/6/21.
//

#include "regex_repository.h"

#include <iostream>
#include <fstream>
#include <memory>
#include <nlohmann/json.hpp>

using nlohmann::json;

rereuse::db::RegexRepository::RegexRepository(const std::string &path) {
    std::ifstream db_file(path);
    if (db_file.is_open()) {
        std::unordered_set<std::string> unique_patterns;
        std::string line;
        int line_count = 0;
        while (std::getline(db_file, line)) {
            line_count++;
            std::string regex_pattern;
            try {
                auto line_obj = json::parse(line);
                regex_pattern = line_obj["pattern"];
                unique_patterns.insert(regex_pattern);
            } catch (json::parse_error &err) {
                // std::cerr << "JSON error while parsing on line " << line_count << ": " << line << std::endl;
            }
        }

        for (const auto &pattern : unique_patterns) {
            RE2::Options re2_opts;
            re2_opts.set_log_errors(false);
            auto regex = std::make_shared<RE2>(pattern, re2_opts);
            if (regex->ok()) {
                this->regexes.push_back(std::move(regex));
            }
        }
    } else {
        std::cerr << "Could not open db file" << std::endl;
    }
}

std::unordered_set<std::string>
rereuse::db::RegexRepository::query(const std::shared_ptr<rereuse::query::BaseRegexQuery> &query) {

    std::unordered_set<std::string> matching_patterns;
    for (const auto &regex : this->regexes) {
        if (query->test(*regex)) {
            matching_patterns.insert(regex->pattern());
        }
    }

    return matching_patterns;
}
