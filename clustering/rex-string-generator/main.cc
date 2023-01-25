//
// Created by charlie on 1/25/23.
//

#include <iostream>
#include "nlohmann/json.hpp"

#include "rex_wrapper.h"
#include "librereuse/db/pattern_reader.h"

#define WINE_PATH "/usr/bin/wine"
#define REX_PATH "/home/cmsale/Rex.exe"

static constexpr std::string_view wine_path(WINE_PATH);
static constexpr std::string_view rex_path(REX_PATH);

int main(int argc, char **argv) {

    if (argc < 3) {
        std::cerr << "usage: tool <input path to objects> <path to strings output>" << std::endl;
        return 1;
    }

    // Read in all the regexes
    auto input_regexes = rereuse::db::read_patterns_from_path(argv[1]);

    // Setup up rex
    RexWrapper wrapper(rex_path.data(), wine_path.data());

    // For each regex, generate 100 rex strings for each
    std::unordered_map<std::string, std::vector<std::string>> rex_strings;
    for (auto &input_regex : input_regexes) {
        try {
            auto generated_strings = wrapper.generate_strings(input_regex, 100);
            // try moving all the resources into the map to reduce memory issues??
            auto pair = std::make_pair(std::move(input_regex), std::move(generated_strings));
            rex_strings.insert(std::move(pair));
        } catch (std::runtime_error &exe) {
            // skip the ones that don't work
            continue;
        }
    }

    // Serialize and output
    nlohmann::json output_obj = rex_strings;

    std::ofstream output(argv[2]);
    output << output_obj << std::endl;

    // DONE

    return 0;
}
