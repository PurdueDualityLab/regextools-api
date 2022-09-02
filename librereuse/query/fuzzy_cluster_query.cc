//
// Created by charlie on 9/7/21.
//

#include "fuzzy_cluster_query.h"

rereuse::query::FuzzyClusterQuery::FuzzyClusterQuery(const std::unordered_set<std::string> &positive,
                                                     const std::unordered_set<std::string> &negative)
                                                     : positive(positive),
                                                       negative(negative) {
}

std::unordered_set<std::string>
rereuse::query::FuzzyClusterQuery::query(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration) {

    std::unordered_set<std::string> potential_matches;
    auto matches_inserter = std::inserter(potential_matches, potential_matches.begin());

    auto start = std::chrono::high_resolution_clock::now();
    for (const auto &pos : positive) {
        std::vector<int> hits;
        // If one completely doesn't match, then short circuit
        if (!cluster->get_regex_set().Match(pos, &hits)) {
            return {};
        }
    }

    for (const auto &neg : negative) {
        std::vector<int> hits;
        if (cluster->get_regex_set().Match(neg, &hits))
        {
            // If (somehow) everything is a match, then short circuit
            if (hits.size() == cluster->get_size()) {
                return {};
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    if (duration) {
        *duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }

    return {"Maybe"};
}

bool rereuse::query::FuzzyClusterQuery::test(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration) {
    std::unordered_set<std::string> potential_matches;
    auto matches_inserter = std::inserter(potential_matches, potential_matches.begin());

    auto start = std::chrono::high_resolution_clock::now();
    for (const auto &pos : positive) {
        std::vector<int> hits;
        // If one completely doesn't match, then short circuit
        if (!cluster->get_regex_set().Match(pos, &hits)) {
            return false;
        }
    }

    for (const auto &neg : negative) {
        std::vector<int> hits;
        if (cluster->get_regex_set().Match(neg, &hits))
        {
            // If (somehow) everything is a match, then short circuit
            if (hits.size() == cluster->get_size()) {
                return false;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    if (duration) {
        *duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }

    return true;
}
