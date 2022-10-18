//
// Created by charlie on 9/2/21.
//

#ifndef _BASECLUSTERQUERY_H
#define _BASECLUSTERQUERY_H

#include <memory>
#include <unordered_set>
#include "../db/cluster.h"

namespace rereuse::query {
    class BaseClusterQuery {
    public:
        virtual ~BaseClusterQuery() = default;

        virtual bool test(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration) = 0;
        virtual std::unordered_set<std::string> query(const std::shared_ptr<rereuse::db::Cluster> &cluster, std::chrono::microseconds *duration, double *average_match_vector_length) = 0;
    };
}

#endif //_BASECLUSTERQUERY_H
