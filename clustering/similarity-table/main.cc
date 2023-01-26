//
// Created by charlie on 1/25/23.
//

#include <iostream>
#include <fstream>
#include <thread>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include "similarity_table.h"
#include "regex_entity.h"

int main(int argc, char **argv) {

    if (argc < 3) {
        std::cerr << "usage: tool <input rex strings> <output path to ABC file>" << std::endl;
        return 1;
    }

    // get the input file
    std::ifstream input(argv[1]);
    nlohmann::json input_object;
    input >> input_object;

    // Make entities out of everything
    std::vector<RegexEntity> entities;
    for (auto &[regex, strings] : input_object.items()) {
        entities.emplace_back(regex, strings);
    }

    spdlog::info("Built out entities");

    // Build the table
    SimilarityTable table(std::move(entities));

    // compute the matrix and such
    table.parallel_compute(std::thread::hardware_concurrency());
    table.to_similarity_graph();

    // Save the result
    std::ofstream output(argv[2]);
    table.output_abc(output);

    return 0;
}
