//
// Created by charlie on 11/2/22.
//

#include <iostream>
#include <spdlog/spdlog.h>
#include "librereuse/db/regex_repository.h"
#include "performance/arg_parser.h"
#include "librereuse/db/pattern_reader.h"
#include "performance/memory/memory_report.h"

int main(int argc, char **argv) {

    if (argc < 2) {
        std::cerr << "usage: tool <path to benchmarking spec>" << std::endl;
    }

    BenchmarkingSpec spec(argv[1]);
    MemoryReport report("serial");

    // Take semantic clusters
    report.pre_db = get_total_allocated_space();
    std::unique_ptr<rereuse::db::RegexRepository> repo;
    {
        auto semantic_clusters = rereuse::db::read_semantic_clusters(spec.db_file_path);
        spdlog::info("Loaded {} semantic clusters", semantic_clusters.size());

        // Duplicate all the patterns
        auto all_patterns = rereuse::db::unpack_patterns(semantic_clusters);
        spdlog::debug("Unpacked {} patterns from all clusters", all_patterns.size());

        repo = std::make_unique<rereuse::db::RegexRepository>(std::move(all_patterns));
    }
    report.post_db = get_total_allocated_space();

    for (const auto &[query_name, query] : spec.queries) {
        const auto &[_, serial_query] = query;

        auto start = std::chrono::high_resolution_clock::now();
        auto results = repo->query(serial_query);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        spdlog::info("{}: got {} result(s) in {}ms", query_name, results.size(), elapsed.count());
    }

    report.after_query = get_total_allocated_space();

    auto csv = report.create_report();

    std::cout << csv;

    return 0;
}
