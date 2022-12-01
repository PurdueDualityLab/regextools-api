//
// Created by charlie on 10/10/22.
//

#include "arg_parser.h"
#include "benchmarking_report.h"

#include <iostream>
#include <random>
#include <spdlog/spdlog.h>
#include <spdlog/stopwatch.h>
#include "librereuse/db/pattern_reader.h"
#include "librereuse/db/regex_repository.h"
#include "librereuse/db/regex_cluster_repository.h"
#include "librereuse/query/match_query.h"
#include "librereuse/query/query_report.h"

static std::unordered_map<unsigned long, unsigned long>
track_results_clusters(const std::unordered_set<std::string> &results, const rereuse::db::RegexClusterRepository &repo) {
    std::unordered_map<unsigned long, unsigned long> cluster_frequency;
    for (const auto &result : results) {
        auto parent_cluster = repo.get_regex_cluster_idx(result);
        if (parent_cluster) {
            if (cluster_frequency.count(*parent_cluster) > 0) {
                cluster_frequency[*parent_cluster] += 1;
            } else {
                cluster_frequency[*parent_cluster] = 1;
            }
        }
    }

    return cluster_frequency;
}

static rereuse::util::CSV
cluster_info_to_csv(const std::unordered_map<unsigned long, unsigned long> &info) {
    rereuse::util::CSV csv;
    auto id_col = csv.push_col("Cluster Id");
    auto freq_col = csv.push_col("Frequency");
    for (const auto &[cluster_id, count] : info) {
        auto row_idx = csv.push_row();
        csv.cell(row_idx, id_col) << cluster_id;
        csv.cell(row_idx, freq_col) << count;
    }

    return csv;
}

static std::vector<rereuse::query::QueryReport> measure_no_clusters(rereuse::db::RegexRepository &serial_repo, const std::unique_ptr<rereuse::query::BaseRegexQuery> &query, unsigned int count) {
    std::vector<rereuse::query::QueryReport> reports(count);
    auto positive_count = dynamic_cast<rereuse::query::MatchQuery*>(query.get())->get_positive().size();
    auto negative_count = dynamic_cast<rereuse::query::MatchQuery*>(query.get())->get_negative().size();

    unsigned int i = 0;
    for (auto & it : reports) {
        rereuse::query::QueryReport report;

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

static std::vector<rereuse::query::QueryReport> measure_clusters(const rereuse::db::RegexClusterRepository &repo, const std::unique_ptr<rereuse::query::BaseClusterQuery> &query, unsigned int count) {
    std::vector<rereuse::query::QueryReport> reports(count);
    auto positive_count = dynamic_cast<rereuse::query::ClusterMatchQuery*>(query.get())->positive_examples_count();
    auto negative_count = dynamic_cast<rereuse::query::ClusterMatchQuery*>(query.get())->negative_examples_count();
    for (auto & it : reports) {
        spdlog::stopwatch sw;
        spdlog::info("Start measuring cluster query... {}", sw);
        auto start = std::chrono::high_resolution_clock::now();
        auto results = repo.deep_query(query);
        auto end = std::chrono::high_resolution_clock::now();
        spdlog::info("Done... {:.3}", sw);

        rereuse::query::QueryReport report(std::move(results));
        report.total_elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        report.positive_examples_count = positive_count;
        report.negative_examples_count = negative_count;
        report.result_cluster_info = track_results_clusters(report.results, repo);
#if 0
        unsigned long result_count = 0;
        for (const auto &[_, cluster_count] : report.result_cluster_info)
            result_count += cluster_count;

        assert(result_count == report.result_count());
#endif

#if 0
        auto csv = cluster_info_to_csv(report.result_cluster_info);
        spdlog::info("INFO TAB");
        std::cout << csv << std::endl;
#endif
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
