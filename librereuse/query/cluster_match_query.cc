//
// Created by charlie on 9/2/21.
//

#include "cluster_match_query.h"
#include "librereuse/util/bitmap.h"
#include "librereuse/util/stats.h"

#include <utility>
#include <numeric>

rereuse::query::ClusterMatchQuery::ClusterMatchQuery(std::unordered_set<std::string> positive,
                                                     std::unordered_set<std::string> negative)
                                                     : positive(std::move(positive)),
                                                       negative(std::move(negative)) {
}

std::unordered_set<std::string>
rereuse::query::ClusterMatchQuery::query(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration, double *average_match_vector_length) {

    auto &set = cluster->get_regex_set();

    std::vector<unsigned long> vector_lengths;

    rereuse::util::Bitmap matching_regexes(cluster->get_size(), true /* fill with 1s */);
    // TODO: there is hopefully room for optimization here

    auto start = std::chrono::high_resolution_clock::now();
    for (const auto &pos : this->positive) {
        std::vector<int> positive_indices;
        if (set.Match(pos, &positive_indices)) {
            vector_lengths.push_back(positive_indices.size());
            rereuse::util::Bitmap hits(matching_regexes.size());
            std::for_each(positive_indices.cbegin(), positive_indices.cend(), [&hits](const int &idx) { hits.set(idx, true); });
            matching_regexes &= hits;
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
            rereuse::util::Bitmap hits(matching_regexes.size(), true);
            std::for_each(positive_indices.cbegin(), positive_indices.cend(), [&hits](const int &idx) { hits.set(idx, false); });

            matching_regexes &= hits;
        }
    }
    auto end = std::chrono::high_resolution_clock::now();

    std::unordered_set<std::string> patterns;
    auto cluster_patterns = cluster->get_patterns();
    for (unsigned long idx = 0; idx < matching_regexes.size(); idx++) {
        if (matching_regexes.get(idx)) {
            patterns.insert(cluster_patterns.at(idx));
        }
    }

    if (duration != nullptr) {
        // If we want to keep track of the duration, put it here
        *duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    }

    if (average_match_vector_length != nullptr) {
        *average_match_vector_length = rereuse::util::mean(vector_lengths.cbegin(), vector_lengths.cend());
    }

    return patterns;
}

bool rereuse::query::ClusterMatchQuery::test(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration) {
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
        } else {
            // Reset this after each invocation
            hits.clear();
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


