//
// Created by charlie on 11/10/22.
//

#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>

#include "librereuse/db/pattern_reader.h"

int main(int argc, char **argv) {

    if (argc < 3) {
        std::cerr << "usage: cluster-randomizer <input clusters path> <output path>" << std::endl;
        return 1;
    }

    auto input_clusters = rereuse::db::read_semantic_clusters(argv[1]);
    auto cluster_size = input_clusters.front()->get_entities().size();

    auto patterns = rereuse::db::unpack_patterns(input_clusters);

    auto randomized_clusters = rereuse::db::randomize_clusters(patterns, cluster_size);

    nlohmann::json randomized_cluster_patterns;
    for (const auto &cluster : randomized_clusters) {
        auto cluster_patterns = cluster->get_entities();
        randomized_cluster_patterns.push_back(std::move(cluster_patterns));
    }

    std::ofstream output(argv[2]);

    output << randomized_cluster_patterns;

    return 0;
}
