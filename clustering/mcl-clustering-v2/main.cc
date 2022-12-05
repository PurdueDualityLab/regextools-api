//
// Created by charlie on 12/3/22.
//

#include <iostream>
#include <filesystem>
#include <spdlog/spdlog.h>
#include "librereuse/db/pattern_reader.h"
#include "./better_similarity_table/rex_similarity_scorer.h"
#include "./better_similarity_table/similarity_table.h"

int main(int argc, char **argv) {

    spdlog::set_level(spdlog::level::debug);

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
    // NOTE the shared_ptr here is very deliberate. This is to ensure that the string's file associated with the
    // object doesn't get cleaned up. When moving, the original delete constructor will get called, and so the
    // strings file gets cleaned up. The better thing to do is to manually add in ref counting to the object and only
    // delete the strings file once the ref count hits zero
    std::vector<std::shared_ptr<RexSimilarityScorer>> scorers;
    for (const auto &regex : regexes) {
        try {
            auto scorer = std::make_shared<RexSimilarityScorer>(regex, id, wrapper);
            scorers.push_back(std::move(scorer));
            id++;
        } catch (std::runtime_error &err) {
            // std::cerr << "Skipping "
            spdlog::warn("Skipping regex /{}/: {}", regex, err.what());
        }
    }

    SimilarityTable<RexSimilarityScorer> similarity_table(std::move(scorers));
    similarity_table.compute();
    similarity_table.to_similarity_graph();

    std::string output = "clusters.abc";
    similarity_table.to_abc(output);

    return 0;
}
