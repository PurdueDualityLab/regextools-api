//
// Created by charlie on 8/31/21.
//

#include <fstream>
#include <random>
#include <vector>
#include <nlohmann/json.hpp>
#include "regex_cluster_repository.h"
#include "../query/match_query.h"

using nlohmann::json;

rereuse::db::RegexClusterRepository::RegexClusterRepository()
: maxClusterSize(0) {}

/* TODO change ctor to take iterators instead of specific data structures. This seems like the more general way */
rereuse::db::RegexClusterRepository::RegexClusterRepository(int maxClusterSize,
                                                            const std::unordered_set<std::string> &patterns)
                                                            : maxClusterSize(maxClusterSize) {
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

    // Compile all of the clusters
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
}

rereuse::db::RegexClusterRepository::RegexClusterRepository(int maxClusterSize, const std::string &path)
        : maxClusterSize(maxClusterSize) {

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

std::unordered_set<std::string>
rereuse::db::RegexClusterRepository::query(const std::shared_ptr<rereuse::query::BaseClusterQuery> &query,
                                           int *skipped_clusters,
                                           std::vector<std::chrono::microseconds> *test_times,
                                           std::vector<std::chrono::microseconds> *query_times) const {
    std::unordered_set<std::string> combined_results;
    for (const auto &cluster : this->clusters) {
        std::chrono::microseconds test_duration, query_duration;
        bool query_happened = false;
        if (query->test(cluster, &test_duration)) {
            auto results = query->query(cluster, &query_duration);
            query_happened = true;
            // Move all the results to combined results
            std::move(results.begin(),  results.end(), std::inserter(combined_results, combined_results.begin()));
        } else {
            if (skipped_clusters) {
                *skipped_clusters += 1;
            }
        }

        if (test_times) {
            test_times->push_back(test_duration);
        }

        if (query_times && query_happened) {
            query_times->push_back(query_duration);
        }
    }

    return combined_results;
}

bool rereuse::db::RegexClusterRepository::add_cluster(std::unique_ptr<Cluster> cluster) {
    std::shared_ptr<Cluster> new_cluster_ptr = std::move(cluster);
    bool compiled = new_cluster_ptr->compile();
    if (compiled) {
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
