//
// Created by charlie on 8/30/21.
//

#include "match_query.h"
#include "miss_score.h"

rereuse::query::MatchQuery::MatchQuery(const std::unordered_set<std::string> &positive,
                                       std::unordered_set<std::string> &negative, int maxMisses)
                                       : BaseRegexQuery(new MissScore(maxMisses)),
                                       positive(positive),
                                       negative(negative) {
}

rereuse::query::MatchQuery::MatchQuery(const std::unordered_set<std::string> &positive,
                                       std::unordered_set<std::string> &negative)
                                       : BaseRegexQuery(new MissScore(0)),
                                       positive(positive),
                                       negative(negative) {
}

double rereuse::query::MatchQuery::score(const RE2 &regex) {

    int tp = 0, tn = 0, fp = 0, fn = 0;

    for (const auto &pos : this->positive) {
        if (RE2::FullMatch(pos, regex)) {
            // If matches a positive, that's a true positive
            tp++;
        } else {
            // That's a false positive
            fp++;
        }
    }

    for (const auto &neg : this->negative) {
        if (!RE2::FullMatch(neg, regex)) {
            // If matches a positive, that's a true positive
            tn++;
        } else {
            // That's a false positive
            fn++;
        }
    }

    return this->scorer->score(tp, tn, fp, fn);
}

const std::unordered_set<std::string> &rereuse::query::MatchQuery::get_positive() const {
    return positive;
}

const std::unordered_set<std::string> &rereuse::query::MatchQuery::get_negative() const {
    return negative;
}


