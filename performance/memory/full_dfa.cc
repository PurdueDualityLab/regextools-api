//
// Created by charlie on 11/10/22.
//

#include <iostream>
#include <spdlog/spdlog.h>
#include <nlohmann/json.hpp>
#include <fstream>

#include "re2/re2.h"
#include "re2/set.h"

#include "performance/memory/memory_report.h"
#include "malloc_info_parser.h"

int main(int argc, char **argv) {

    spdlog::set_level(spdlog::level::debug);

    if (argc < 3) {
        std::cerr << "usage: <cluster file> <size output path>" << std::endl;
        return 1;
    }

    std::vector<std::string> patterns;
    {
        nlohmann::json cluster_obj;
        std::string input_path(argv[1]);
        std::ifstream input(input_path);
        input >> cluster_obj;

        for (const auto &pattern : cluster_obj) {
            auto pattern_string = pattern.get<std::string>();
            patterns.push_back(pattern_string);
        }
    }
    spdlog::info("Got {} patterns", patterns.size());

    re2::RE2::Options opts;
    auto max_mem_size = static_cast<int64_t>(1073741824) * 6; // 1GiB * num of GiBs
    opts.set_max_mem(max_mem_size);
    re2::RE2::Set set(opts, re2::RE2::ANCHOR_BOTH);

    // load in the patterns
    for (auto &pattern : patterns) {
        // Just gonna assume these all work because they're supposed to be valid
        set.Add(pattern, nullptr);
    }
    spdlog::info("Loaded patterns into set");

    // Now, compile
    spdlog::info("Starting to compile. This will take a looooong time...");
    // auto before_mem = get_total_allocated_space();
    auto before_alt = heap_get_allocated_bytes();
    bool success = false;
    set.Compile(true, &success);
    // auto after_mem = get_total_allocated_space();
    auto after_alt = heap_get_allocated_bytes();
    spdlog::info("Compilation done. Build status: {}", success);

    auto total_allocated_space = after_alt - before_alt;

    spdlog::info("Build out DFA size: {}kb", total_allocated_space / 1'000);

    std::ofstream output(argv[2]);
    output << (total_allocated_space / 1'000) << std::endl; // only store KB
    output << (success ? "true" : "false") << std::endl;

    auto backup_memory = after_alt - before_alt;
    spdlog::info("Backup result: {}kb", backup_memory / 1'000);

    return 0;
}
