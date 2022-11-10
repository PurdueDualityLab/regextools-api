//
// Created by charlie on 10/10/22.
//

#include "arg_parser.h"
#include "query_report.h"

#include <iostream>
#include <random>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include "librereuse/db/pattern_reader.h"
#include "librereuse/db/regex_repository.h"
#include "librereuse/db/regex_cluster_repository.h"
#include "librereuse/query/match_query.h"

static std::vector<QueryReport> measure_no_clusters(rereuse::db::RegexRepository &serial_repo, const std::unique_ptr<rereuse::query::BaseRegexQuery> &query, unsigned int count) {
    std::vector<QueryReport> reports(count);
    auto positive_count = dynamic_cast<rereuse::query::MatchQuery*>(query.get())->get_positive().size();
    auto negative_count = dynamic_cast<rereuse::query::MatchQuery*>(query.get())->get_negative().size();

    unsigned int i = 0;
    for (auto & it : reports) {
        QueryReport report;

        spdlog::stopwatch sw;
        spdlog::debug("Start serial query {}... {}", i, sw);
        auto start = std::chrono::high_resolution_clock::now();
        auto results = serial_repo.query(query);
        auto end = std::chrono::high_resolution_clock::now();
        spdlog::debug("{} Done... {:.3}", i, sw);

        std::move(results.begin(), results.end(), std::inserter(report.results, report.results.begin()));
        report.total_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        report.positive_examples_count = positive_count;
        report.negative_examples_count = negative_count;
        it = std::move(report);
        i++;
    }

    return reports;
}

static std::vector<QueryReport> measure_clusters(const rereuse::db::RegexClusterRepository &repo, const std::unique_ptr<rereuse::query::BaseClusterQuery> &query, unsigned int count) {
    std::vector<QueryReport> reports(count);
    auto positive_count = dynamic_cast<rereuse::query::ClusterMatchQuery*>(query.get())->positive_examples_count();
    auto negative_count = dynamic_cast<rereuse::query::ClusterMatchQuery*>(query.get())->negative_examples_count();
    for (auto & it : reports) {
        QueryReport report;
        spdlog::stopwatch sw;
        spdlog::info("Start measuring cluster query... {}", sw);
        auto start = std::chrono::high_resolution_clock::now();
        auto results = repo.query(query, &report.skipped_clusters, &report.median_test_fail_time, &report.median_test_pass_time, &report.median_drill_time,
                                  &report.average_vec_size);
        auto end = std::chrono::high_resolution_clock::now();
        spdlog::info("Done... {:.3}", sw);

        std::move(results.begin(), results.end(), std::inserter(report.results, report.results.begin()));
        report.total_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        report.positive_examples_count = positive_count;
        report.negative_examples_count = negative_count;
        it = std::move(report);
    };

    return reports;
}

int main(int argc, char **argv) {

    // spdlog::set_level(spdlog::level::debug);

    ProgramArgs args(argc, argv);

    if (args.showHelp) {
        std::cout << "Help!" << std::endl;
        return 0;
    }

    // Break down the spec file into a spec object
    BenchmarkingSpec bench_spec(args.spec_file);

    // Take semantic clusters
    auto semantic_clusters = rereuse::db::read_semantic_clusters(bench_spec.db_file_path);
    spdlog::info("Loaded {} semantic clusters", semantic_clusters.size());

    // Duplicate all the patterns
    auto all_patterns = unpack_patterns(semantic_clusters);
    spdlog::debug("Unpacked {} patterns from all clusters", all_patterns.size());

    // Create randomized clusters
    auto randomized_clusters = rereuse::db::randomize_clusters(all_patterns, 100);
    spdlog::debug("Create {} new randomized clusters", randomized_clusters.size());

    rereuse::db::RegexRepository serial_repo(all_patterns);

    rereuse::db::RegexClusterRepository semantic_repo;
    for (auto &cluster : semantic_clusters) {
        semantic_repo.add_cluster(std::move(cluster));
    }

    rereuse::db::RegexClusterRepository random_repo;
    for (auto &cluster : randomized_clusters) {
        random_repo.add_cluster(std::move(cluster));
    }

    std::unordered_map<std::string, BenchmarkReport> reports;

    for (const auto &[name, queries] : bench_spec.queries) {

        const auto &[cluster_query, serial_query] = queries;

        spdlog::info("{}: Measuring serial report...", name);
        auto serial_report = measure_no_clusters(serial_repo, serial_query, 6);
        spdlog::info("{}: Measuring random report...", name);
        auto random_report = measure_clusters(random_repo, cluster_query, 6);
        spdlog::info("{}: Measuring semantic report...", name);
        auto semantic_report = measure_clusters(semantic_repo, cluster_query, 6);

        BenchmarkReport report(semantic_clusters.size());
        report.add_query_report("Serial (1st)", serial_report[0]);
        report.add_query_report("Random (1st)", random_report[0]);
        report.add_query_report("Semantic (1st)", semantic_report[0]);
        serial_report.erase(serial_report.begin());
        random_report.erase(random_report.begin());
        semantic_report.erase(semantic_report.begin());
        report.add_query_report("Serial (median n=6-1)", median_query_report(serial_report));
        report.add_query_report("Random (median n=6-1)", median_query_report(random_report));
        report.add_query_report("Semantic (median n=6-1)", median_query_report(semantic_report));

        reports[name] = std::move(report);
    }

    for (const auto &[name, report] : reports) {
        std::cout << '\n' << name << std::endl;
        std::cout << report;
    }

    return 0;
}
