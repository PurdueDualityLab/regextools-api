//
// Created by charlie on 4/4/23.
//

#ifndef REGEXTOOLS_USE_CASE_H
#define REGEXTOOLS_USE_CASE_H

#include <string>
#include <unordered_set>
#include <vector>
#include <memory>

#include "regex_entity.h"
#include "librereuse/query/cluster_match_query.h"

class UseCase {
public:
    UseCase(std::string regex, std::vector<std::string> unclassified);
    UseCase(std::string regex, std::vector<std::string> positive, std::vector<std::string> negative);

    /**
     * Create a query out of the examples in this use case
     * @return A cluster query that can be used to query the database
     */
    std::unique_ptr<rereuse::query::ClusterMatchQuery> create_query() const;

    void add_result(const RegexEntity &entity);

private:
    std::string regex; // the regex that started the entire use case
    std::unordered_set<std::string> positive; // the positive strings that define this use-case
    std::unordered_set<std::string> negative; // the negative strings that define this use-case
    std::vector<RegexEntity> results; // the results for this use case. May be empty
};


#endif //REGEXTOOLS_USE_CASE_H
