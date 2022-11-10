//
// Created by charlie on 8/31/21.
//

#ifndef _REGEXCLUSTERREPOSITORY_H
#define _REGEXCLUSTERREPOSITORY_H

#include <iostream>
#include <memory>
#include <vector>
#include <thread>
#include "cluster.h"
#include "../query/base_regex_query.h"
#include "../query/cluster_match_query.h"

namespace rereuse::db {
    /**
     * Represents a clustered regex repository
     */
    class RegexClusterRepository {
    public:
        explicit RegexClusterRepository();
        explicit RegexClusterRepository(int maxClusterSize, const std::unordered_set<std::string> &patterns);
        explicit RegexClusterRepository(int maxClusterSize, const std::string& path);

        virtual ~RegexClusterRepository() = default;

        int cluster_count() const { return this->clusters.size(); }
        int pattern_count() const;
        bool add_cluster(std::unique_ptr<Cluster> cluster);
        void shuffle_clusters();

        void set_skip_test_optimization() { this->skipTestOpt = true; }

        const std::vector<std::shared_ptr<Cluster>> &get_clusters();

        std::optional<unsigned long> get_regex_cluster_idx(const std::string &pattern) const;

        virtual std::unordered_set<std::string>
        query(const std::unique_ptr<rereuse::query::BaseClusterQuery> &query) const {
            return this->query(query, nullptr, nullptr, nullptr, nullptr, nullptr);
        }

        virtual std::unordered_set<std::string>
        query(const std::unique_ptr<rereuse::query::BaseClusterQuery> &query, unsigned long *skipped_clusters,
              std::chrono::microseconds *median_test_fail_time, std::chrono::microseconds *median_test_pass_time,
              std::chrono::microseconds *median_drill_time, double *average_match_vector_size) const;

    protected:
        int maxClusterSize;
        std::vector<std::shared_ptr<Cluster>> clusters;
        bool skipTestOpt;
    };
}

#endif //_REGEXCLUSTERREPOSITORY_H
