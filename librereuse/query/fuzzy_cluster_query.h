//
// Created by charlie on 9/7/21.
//

#ifndef _FUZZYCLUSTERQUERY_H
#define _FUZZYCLUSTERQUERY_H

#include "librereuse/query/base_cluster_query.h"

namespace rereuse::query {
    /**
     * Determines a set of regexes that <i>might</i> match
     */
    class FuzzyClusterQuery : public BaseClusterQuery {
    public:
        explicit FuzzyClusterQuery(std::unordered_set<std::string> positive, std::unordered_set<std::string> negative);

        std::vector<db::RegexEntity> query(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration = nullptr, double *average_vector_size = nullptr) override;

        bool test(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration = nullptr) override;

    private:
        std::unordered_set<std::string> positive;
        std::unordered_set<std::string> negative;
    };
}

#endif //_FUZZYCLUSTERQUERY_H
