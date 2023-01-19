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
#include "librereuse/db/regex_entity.h"

namespace rereuse::db {
    class RegexRepository {
    public:
        explicit RegexRepository(const std::vector<RegexEntity>& patterns);

        int size() { return this->regexes.size(); }

        std::vector<RegexEntity> query(const std::unique_ptr<query::BaseRegexQuery> &query);

    private:
        std::vector<std::unique_ptr<RE2>> regexes;
        std::vector<RegexEntity> entities;
    };
}

#endif //_REGEXREPOSITORY_H
