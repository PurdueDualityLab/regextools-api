//
// Created by charlie on 12/3/22.
//

#include <iostream>
#include <filesystem>
#include "librereuse/db/pattern_reader.h"
#include "./better_similarity_table/rex_similarity_scorer.h"
#include "./better_similarity_table/similarity_table.h"

int main(int argc, char **argv) {

    if (argc < 2) {
        std::cerr << "usage: mcl-clustering-v2 <regex objects path>" << std::endl;
        return 1;
    }

    // Set up a rex wrapper
    RexWrapper wrapper("/home/cmsale/Rex.exe", "/usr/bin/wine"); // TODO fill this out

    // The first arg is the path to the database file to cluster
    auto regexes = rereuse::db::read_patterns_from_path(argv[1]);

    // Make scorers for everything
    unsigned long id = 0;
    std::vector<RexSimilarityScorer> scorers;
    for (const auto &regex : regexes) {
        scorers.emplace_back(regex, id++, wrapper);
    }

    SimilarityTable<RexSimilarityScorer> similarity_table(std::move(scorers));
    similarity_table.compute();
    similarity_table.to_similarity_graph();

    std::string output = "clusters.abc";
    similarity_table.to_abc(output);

    return 0;
}
