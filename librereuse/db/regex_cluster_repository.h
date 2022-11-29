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
#include "../query/query_report.h"

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
            auto result = this->deep_query(query);
            return std::move(result.results);
        }

        query::QueryResult
        deep_query(const std::unique_ptr<rereuse::query::BaseClusterQuery> &query) const;

    protected:
        int maxClusterSize;
        std::vector<std::shared_ptr<Cluster>> clusters;
        bool skipTestOpt;
    };
}

#endif //_REGEXCLUSTERREPOSITORY_H
