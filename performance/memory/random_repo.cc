//
// Created by charlie on 11/2/22.
//

#include <iostream>
#include <spdlog/spdlog.h>
#include "performance/arg_parser.h"
#include "librereuse/db/pattern_reader.h"
#include "librereuse/db/regex_cluster_repository.h"
#include "performance/memory/memory_report.h"

int main(int argc, char **argv) {

    if (argc < 2) {
        std::cerr << "usage: tool <path to benchmarking spec>" << std::endl;
    }

    BenchmarkingSpec spec(argv[1]);
    MemoryReport report("random");

    // Take semantic clusters
    report.pre_db = get_total_allocated_space();
    auto repo = std::make_unique<rereuse::db::RegexClusterRepository>();
    repo->set_skip_test_optimization();
    {
        auto semantic_clusters = rereuse::db::read_semantic_clusters(spec.db_file_path);
        spdlog::info("Loaded {} semantic clusters", semantic_clusters.size());

        // Duplicate all the patterns
        auto all_patterns = rereuse::db::unpack_patterns(semantic_clusters);
        spdlog::debug("Unpacked {} patterns from all clusters", all_patterns.size());

        auto random_clusters = rereuse::db::randomize_clusters(all_patterns, semantic_clusters[0]->get_size());

        for (auto &cluster : random_clusters) {
            repo->add_cluster(std::move(cluster));
        }
    }
    report.post_db = get_total_allocated_space();

    for (const auto &[query_name, query] : spec.queries) {
        const auto &[cluster_query, _] = query;

        auto start = std::chrono::high_resolution_clock::now();
        auto results = repo->query(cluster_query);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        spdlog::info("{}: got {} result(s) in {}ms", query_name, results.size(), elapsed.count());
    }

    report.after_query = get_total_allocated_space();

    auto csv = report.create_report();
    std::cout << csv;

    return 0;
}
