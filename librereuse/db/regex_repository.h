//
// Created by charlie on 9/6/21.
//

#ifndef _REGEXREPOSITORY_H
#define _REGEXREPOSITORY_H

#include <istream>
#include <string>
#include <unordered_set>
#include <memory>
#include "re2/re2.h"

#include "../query/base_regex_query.h"

namespace rereuse::db {
    class RegexRepository {
    public:
        explicit RegexRepository(const std::string &path);
        explicit RegexRepository(const std::vector<std::string>& patterns);

        int size() { return this->regexes.size(); }

        std::unordered_set<std::string> query(const std::unique_ptr<query::BaseRegexQuery> &query);

    private:
        std::vector<std::unique_ptr<re2::RE2>> regexes;
    };
}

#endif //_REGEXREPOSITORY_H
