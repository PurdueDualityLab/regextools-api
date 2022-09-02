//
// Created by charlie on 9/7/21.
//

#ifndef _FUZZYCLUSTERQUERY_H
#define _FUZZYCLUSTERQUERY_H

// #include "base_cluster_query.h"
#include "librereuse/query/base_cluster_query.h"

namespace rereuse::query {
    /**
     * Determines a set of regexes that <i>might</i> match
     */
    class FuzzyClusterQuery : public BaseClusterQuery {
    public:
        explicit FuzzyClusterQuery(const std::unordered_set<std::string> &positive, const std::unordered_set<std::string> &negative);

        std::unordered_set<std::string> query(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration = nullptr) override;

        bool test(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration = nullptr) override;

    private:
        std::unordered_set<std::string> positive;
        std::unordered_set<std::string> negative;
    };
}

#endif //_FUZZYCLUSTERQUERY_H
