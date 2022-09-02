//
// Created by charlie on 9/2/21.
//

#include "cluster_match_query.h"

#include <utility>
#include <numeric>

rereuse::query::ClusterMatchQuery::ClusterMatchQuery(std::unordered_set<std::string> positive,
                                                     std::unordered_set<std::string> negative)
                                                     : positive(std::move(positive)),
                                                       negative(std::move(negative)) {
}

std::unordered_set<std::string>
rereuse::query::ClusterMatchQuery::query(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration) {

    auto &set = cluster->get_regex_set();

    std::unordered_set<int> matched;

    auto start = std::chrono::high_resolution_clock::now();
    for (const auto &pos : this->positive) {
        std::vector<int> positive_indices;
        if (set.Match(pos, &positive_indices)) {
            if (matched.empty()) {
                // If matched is empty, then push all of these values in
                std::copy(positive_indices.cbegin(), positive_indices.cend(), std::inserter(matched, matched.begin()));
            } else {
                // Find the intersection between these indices and the already matched ones
                std::unordered_set<int> intersection;
                std::set_intersection(matched.cbegin(),  matched.cend(), positive_indices.cbegin(),  positive_indices.cend(), std::inserter(intersection, intersection.begin()));

                // Set the intersection to equal
                matched = intersection;
            }
        } else {
            // Nothing matched, so return empty set
            auto end = std::chrono::high_resolution_clock::now();
            if (duration != nullptr) {
                // If we want to keep track of the duration, put it here
                *duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            }
            return {};
        }
    }

    /* Kinda the same as before, except this time the ones that don't match are the good ones
     * Switch the roles basically */
    for (const auto &neg : this->negative) {
        std::vector<int> positive_indices;
        if (set.Match(neg, &positive_indices)) {
            // Something matched, so indices must be removed from matched
            for (auto matched_index = matched.begin(); matched_index != matched.end();) {
                if (std::count(positive_indices.cbegin(), positive_indices.cend(), *matched_index)) {
                    matched_index = matched.erase(matched_index);
                } else {
                    matched_index++;
                }
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::unordered_set<std::string> patterns;
    for (const auto &index : matched) {
        // For each index, get the pattern at that index
        patterns.insert(cluster->get_patterns().at(index));
    }

    if (duration != nullptr) {
        // If we want to keep track of the duration, put it here
        *duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }

    return patterns;
}

bool rereuse::query::ClusterMatchQuery::test(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration) {
    std::unordered_set<std::string> potential_matches;
    auto matches_inserter = std::inserter(potential_matches, potential_matches.begin());

    auto start = std::chrono::high_resolution_clock::now();
    for (const auto &pos : positive) {
        std::vector<int> hits;
        // If one completely doesn't match, then short circuit
        if (!cluster->get_regex_set().Match(pos, &hits)) {
            auto end = std::chrono::high_resolution_clock::now();

            if (duration != nullptr) {
                *duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            }
            return false;
        }
    }

    for (const auto &neg : negative) {
        std::vector<int> hits;
        if (cluster->get_regex_set().Match(neg, &hits)) {
            // If (somehow) everything is a match, then short circuit
            if (hits.size() == cluster->get_size()) {
                auto end = std::chrono::high_resolution_clock::now();

                if (duration != nullptr) {
                    *duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
                }
                return false;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    if (duration != nullptr) {
        *duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }

    return true;
}

double rereuse::query::ClusterMatchQuery::average_positive_size() const {
    int total_size = std::transform_reduce(this->positive.cbegin(),  this->positive.cend(), 0, std::plus<>(),
            [&](const std::string &str) -> std::string::size_type { return str.size(); });
    return (double) total_size / (double) this->positive.size();
}

double rereuse::query::ClusterMatchQuery::average_negative_size() const {
    int total_size = std::transform_reduce(this->negative.cbegin(),  this->negative.cend(), 0, std::plus<>(),
                                           [&](const std::string &str) -> std::string::size_type { return str.size(); });
    return (double) total_size / (double) this->negative.size();
}


