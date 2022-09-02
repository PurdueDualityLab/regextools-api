//
// Created by charlie on 9/6/21.
//

#ifndef _REGEXREPOSITORY_H
#define _REGEXREPOSITORY_H

#include <istream>
#include <string>
#include <unordered_set>
#include <memory>
#include <re2/re2.h>

#include "../query/base_regex_query.h"

namespace rereuse::db {
    class RegexRepository {
    public:
        explicit RegexRepository(const std::string &path);

        int size() { return this->regexes.size(); }

        std::unordered_set<std::string> query(const std::shared_ptr<rereuse::query::BaseRegexQuery> &query);

    private:
        std::vector<std::shared_ptr<RE2>> regexes;
    };
}

#endif //_REGEXREPOSITORY_H
