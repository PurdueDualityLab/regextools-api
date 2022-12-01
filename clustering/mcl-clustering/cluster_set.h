//
// Created by charlie on 2/10/22.
//

#ifndef _CLUSTER_SET_H
#define _CLUSTER_SET_H

#include <string>
#include <vector>
#include <unordered_set>
#include <ostream>
#include "similarity_table/similarity_table.h"

class ClusterSet {
public:
    explicit ClusterSet(const std::string &cluster_file_path);
    explicit ClusterSet(const std::vector<std::vector<unsigned long>> &clusters);

    void write_patterns(const SimilarityTable &table, std::ostream &out) const;

    const std::vector<std::unordered_set<unsigned long>> &get_clusters() const {
        return clusters;
    }

    friend std::ostream &operator<<(std::ostream &os, const ClusterSet &set);

private:
    std::vector<std::unordered_set<unsigned long>> clusters;
};

#endif //_CLUSTER_SET_H
