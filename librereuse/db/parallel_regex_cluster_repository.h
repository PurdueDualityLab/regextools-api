//
// Created by charlie on 12/21/21.
//

#ifndef _PARALLEL_REGEX_CLUSTER_REPOSITORY_H
#define _PARALLEL_REGEX_CLUSTER_REPOSITORY_H

#include <vector>
#include <thread>

#include "regex_cluster_repository.h"

namespace rereuse::db {
    class ParallelRegexClusterRepository : public rereuse::db::RegexClusterRepository {
    public:
        explicit ParallelRegexClusterRepository(unsigned int processors);
        ParallelRegexClusterRepository(unsigned int processors, int maxClusterSize, const std::string &path);

        std::unordered_set<std::string>
        query(const std::unique_ptr<rereuse::query::BaseClusterQuery> &query) const override;

    private:
        unsigned int processors;
    };
}

#endif //_PARALLEL_REGEX_CLUSTER_REPOSITORY_H
