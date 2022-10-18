//
// Created by charlie on 9/2/21.
//

#ifndef _CLUSTERMATCHQUERY_H
#define _CLUSTERMATCHQUERY_H


#include "base_cluster_query.h"

namespace rereuse::query {
    class ClusterMatchQuery : public BaseClusterQuery {
    public:
        explicit ClusterMatchQuery(std::unordered_set<std::string> positive, std::unordered_set<std::string> negative);

        /**
         * Searches a cluster for patterns that match the query
         * @param cluster Cluster to query
         * @return set of patterns that match the query
         */
        std::unordered_set<std::string> query(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration, double *average_match_vector_length) override;

        /**
         * Determines if a cluster might have a match
         * @param cluster Cluster to be searched
         * @return True if the cluster should be searched
         */
        bool test(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration) override;

        double average_positive_size() const;
        double average_negative_size() const;

        int positive_examples_count() const { return this->positive.size(); }
        int negative_examples_count() const { return this->negative.size(); }

    private:
        std::unordered_set<std::string> positive;
        std::unordered_set<std::string> negative;
    };
}

#endif //_CLUSTERMATCHQUERY_H
