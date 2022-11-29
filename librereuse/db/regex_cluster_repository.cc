//
// Created by charlie on 8/31/21.
//

#include <fstream>
#include <random>
#include <vector>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include "regex_cluster_repository.h"
#include "../query/match_query.h"
#include "librereuse/util/stats.h"

using nlohmann::json;

rereuse::db::RegexClusterRepository::RegexClusterRepository()
: maxClusterSize(0)
, skipTestOpt(false) {}

/* TODO change ctor to take iterators instead of specific data structures. This seems like the more general way */
rereuse::db::RegexClusterRepository::RegexClusterRepository(int maxClusterSize,
                                                            const std::unordered_set<std::string> &patterns)
                                                            : maxClusterSize(maxClusterSize)
                                                            , skipTestOpt(false) {
    for (const auto &pattern : patterns) {
        // If there is an empty cluster...
        if (!this->clusters.empty() && this->clusters.back()->get_size() < this->maxClusterSize) {
            // Get the back cluster
            auto back = this->clusters.back();
            // And push the pattern
            bool added = back->add_pattern(pattern);
#if 0
            if (!added) {
                    std::cerr << "RegexClusterRepository: failed to add pattern to cluster" << std::endl;
                } else {
                    std::cout << "RegexClusterRepository: added pattern to cluster" << std::endl;
                }
#endif
        } else {
            // Otherwise, push a new one
            auto new_cluster = std::make_shared<Cluster>();
            bool successful = new_cluster->add_pattern(pattern);
            if (successful) {
                // If successful, push the new cluster onto the back
                this->clusters.push_back(std::move(new_cluster));
            } else {
                // Ignore the pattern
            }
        }
    }

    // Compile all the clusters
    for (auto it = clusters.begin(); it != clusters.end();) {
        auto cluster = *it;
        if (cluster->compile()) {
            ++it;
        } else {
            // Failed to compile, so remove it from the list
            spdlog::error("RegexClusterRepository:Ctor: Failed to compile cluster");
            it = this->clusters.erase(it);
        }
    }
}

rereuse::db::RegexClusterRepository::RegexClusterRepository(int maxClusterSize, const std::string &path)
        : maxClusterSize(maxClusterSize)
        , skipTestOpt(false) {

    // Open a file to the path
    std::ifstream db_file(path);
    if (db_file.is_open()) {
        // Parse the file line by line
        std::string line;
        while (std::getline(db_file, line)) {
            std::string regex_pattern;
            try {
                auto line_obj = json::parse(line);
                // Get the pattern out of each json object
                regex_pattern = line_obj["pattern"];
                if (regex_pattern.size() > 100) {
                    // Maybe filtering out super long patterns will help fight against the segfaults
                    continue;
                }
            } catch (json::parse_error& err) {
                continue;
            }

            // If there is an empty cluster...
            if (!this->clusters.empty() && this->clusters.back()->get_size() < this->maxClusterSize) {
                // Get the back cluster
                auto back = this->clusters.back();
                // And push the pattern
                bool added = back->add_pattern(regex_pattern);
#if 0
                if (!added) {
                    std::cerr << "RegexClusterRepository: failed to add pattern to cluster" << std::endl;
                } else {
                    std::cout << "RegexClusterRepository: added pattern to cluster" << std::endl;
                }
#endif
            } else {
                // Otherwise, push a new one
                auto new_cluster = std::make_shared<Cluster>();
                bool successful = new_cluster->add_pattern(regex_pattern);
                if (successful) {
                    // If successful, push the new cluster onto the back
                    this->clusters.push_back(std::move(new_cluster));
                    //std::cout << "RegexClusterRepository: added new cluster" << std::endl;
                } else {
                    //std::cerr << "RegexClusterRepository: failed to add new cluster" << std::endl;
                }
            }
        }

        // Compile all of the regex sets
        for (auto it = clusters.begin(); it != clusters.end();) {
            auto cluster = *it;
            if (cluster->compile()) {
                ++it;
            } else {
                // Failed to compile, so remove it from the list
                std::cerr << "Failed to compile cluster" << std::endl;
                it = this->clusters.erase(it);
            }
        }
    } else {
        // TODO throw an exception
    }
}

int rereuse::db::RegexClusterRepository::pattern_count() const {
    int total_patterns = 0;
    for (const auto &cluster : this->clusters) {
        total_patterns += cluster->get_patterns().size();
    }

    return total_patterns;
}

rereuse::query::QueryResult
rereuse::db::RegexClusterRepository::deep_query(const std::unique_ptr<rereuse::query::BaseClusterQuery> &query) const {
    std::vector<double> average_vector_sizes;
    std::unordered_set<std::string> combined_results;
    unsigned long cluster_id = 0;
    std::vector<std::chrono::microseconds> test_hit_times, test_miss_times, drill_times;
    unsigned long skipped_clusters = 0;

    for (const auto &cluster : this->clusters) {
        std::chrono::microseconds test_duration, query_duration;
        double average_vec_size;
        if (skipTestOpt || query->test(cluster, &test_duration)) {
            auto results = query->query(cluster, &query_duration, &average_vec_size);
            average_vector_sizes.push_back(average_vec_size);
            // Move all the results to combined results
            std::copy(results.cbegin(), results.cend(), std::inserter(combined_results, combined_results.begin()));

            test_hit_times.push_back(test_duration);
            drill_times.push_back(query_duration);
        } else {
            spdlog::debug("Skipped cluster #{}", cluster_id);
            skipped_clusters++;
            test_miss_times.push_back(test_duration);
        }

        cluster_id++;
    }

    rereuse::query::QueryResult result(
            std::move(combined_results),
            skipped_clusters,
            rereuse::util::median_duration(test_miss_times.cbegin(), test_miss_times.cend()),
            rereuse::util::median_duration(test_hit_times.cbegin(), test_hit_times.cend()),
            rereuse::util::median_duration(drill_times.cbegin(), drill_times.cend()),
            rereuse::util::mean(average_vector_sizes.cbegin(), average_vector_sizes.cend())
            );

    return result;
}

bool rereuse::db::RegexClusterRepository::add_cluster(std::unique_ptr<Cluster> cluster) {
    static std::atomic<std::size_t> compiled_clusters;
    std::shared_ptr<Cluster> new_cluster_ptr = std::move(cluster);
    // spdlog::info("Compiling cluster {}...", ++compiled_clusters);
    bool compiled = new_cluster_ptr->compile(false);
    // spdlog::info("Cluster {} compiled", compiled_clusters);
    if (compiled) {
#if 0
        new_cluster_ptr->prime();
#endif
        this->clusters.push_back(new_cluster_ptr);
    }

    return compiled;
}

void rereuse::db::RegexClusterRepository::shuffle_clusters() {
    std::shuffle(this->clusters.begin(), this->clusters.end(), std::mt19937(std::random_device()()));
}

const std::vector<std::shared_ptr<rereuse::db::Cluster>> &rereuse::db::RegexClusterRepository::get_clusters() {
    return this->clusters;
}

std::optional<unsigned long> rereuse::db::RegexClusterRepository::get_regex_cluster_idx(const std::string &pattern) const {

    unsigned long idx = 0;
    for (const auto &cluster : this->clusters) {
        if (std::count(cluster->get_patterns().begin(), cluster->get_patterns().end(), pattern) > 0) {
            return {idx};
        } else {
            idx++;
        }
    }

    return {};
}
