//
// Created by charlie on 8/30/21.
//

#ifndef _MATCHQUERY_H
#define _MATCHQUERY_H

#include <string>
#include <unordered_set>
#include "base_regex_query.h"

namespace rereuse::query {
    /**
     * Tests a single regex
     */
    class MatchQuery : public BaseRegexQuery {
    public:
        explicit MatchQuery(const std::unordered_set<std::string>& positive, std::unordered_set<std::string>& negative);
        explicit MatchQuery(const std::unordered_set<std::string>& positive, std::unordered_set<std::string>& negative, int maxMisses);

        double score(const RE2 &regex) override;

        const std::unordered_set<std::string> &get_positive() const;
        const std::unordered_set<std::string> &get_negative() const;

    private:
        std::unordered_set<std::string> positive;
        std::unordered_set<std::string> negative;
    };
}

#endif //_MATCHQUERY_H
