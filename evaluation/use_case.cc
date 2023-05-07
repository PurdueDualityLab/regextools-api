//
// Created by charlie on 4/4/23.
//

#include "use_case.h"


UseCase::UseCase(std::string regex, std::vector<std::string> unclassified)
: regex(std::move(regex)) {
    re2::RE2 pattern(this->regex);
    assert(pattern.ok());

    for (auto &str : unclassified) {
        if (re2::RE2::FullMatch(str, pattern)) {
            this->positive.insert(std::move(str));
        } else {
            this->negative.insert(std::move(str));
        }
    }
}

UseCase::UseCase(std::string regex, std::vector<std::string> positive, std::vector<std::string> negative)
: regex(std::move(regex)) {
    // Move these over
    std::move(positive.begin(), positive.end(), std::inserter(this->positive, this->positive.begin()));
    std::move(negative.begin(), negative.end(), std::inserter(this->negative, this->negative.begin()));
}

std::unique_ptr<rereuse::query::ClusterMatchQuery> UseCase::create_query() const {
    return std::make_unique<rereuse::query::ClusterMatchQuery>(this->positive, this->negative);
}

void UseCase::add_result(const RegexEntity &entity) {
    this->results.push_back(entity);
}
