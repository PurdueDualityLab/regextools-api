//
// Created by charlie on 10/10/22.
//

#include "performance/report.h"
#include "arg_parser.h"
#include "query_report.h"

#include <iostream>
#include <random>
#include <iomanip>
#include <spdlog/spdlog.h>
#include "librereuse/db/pattern_reader.h"
#include "librereuse/db/regex_repository.h"
#include "librereuse/db/regex_cluster_repository.h"
#include "librereuse/query/match_query.h"

std::vector<std::string> unpack_patterns(const std::vector<std::unique_ptr<rereuse::db::Cluster>> &clusters) {
    std::vector<std::string> all_patterns;
    for (const auto &cluster : clusters) {
        const auto &patterns = cluster->get_patterns();
        std::copy(patterns.cbegin(), patterns.cend(), std::back_inserter(all_patterns));
    }

    return all_patterns;
}

std::vector<std::unique_ptr<rereuse::db::Cluster>> randomize_clusters(const std::vector<std::string> &all_patterns_orig, unsigned int expected_cluster_size) {
    // Copy all the patterns
    std::vector<std::string> all_patterns(all_patterns_orig.cbegin(), all_patterns_orig.cend());
    // Shuffle their order
    std::shuffle(all_patterns.begin(), all_patterns.end(), std::mt19937(std::random_device()()));
    spdlog::debug("randomized_cluster: creating clusters of size {}", expected_cluster_size);

    std::vector<std::unique_ptr<rereuse::db::Cluster>> new_clusters;
    std::unordered_set<std::string> cluster_strings;
    for (auto &pattern : all_patterns) {
        // Add this item to the current cluster strings
        cluster_strings.insert(std::move(pattern));

        if (cluster_strings.size() == expected_cluster_size) {
            // This cluster is full
            auto new_cluster = std::make_unique<rereuse::db::Cluster>(cluster_strings);
            // Clear the set
            cluster_strings.clear();

            new_clusters.push_back(std::move(new_cluster));
        }
    }

    return new_clusters;
}

QueryReport measure_no_clusters(const std::vector<std::string> &patterns, const std::unique_ptr<rereuse::query::BaseRegexQuery> &query) {
    QueryReport report;

    rereuse::db::RegexRepository repo(patterns);
    auto start = std::chrono::high_resolution_clock::now();
    auto results = repo.query(query);
    auto end = std::chrono::high_resolution_clock::now();

    std::move(results.begin(), results.end(), std::inserter(report.results, report.results.begin()));
    report.total_elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return report;
}

QueryReport measure_clusters(const rereuse::db::RegexClusterRepository &repo, const std::unique_ptr<rereuse::query::BaseClusterQuery> &query) {
    QueryReport report;
    auto start = std::chrono::high_resolution_clock::now();
    auto results = repo.query(query, &report.skipped_clusters, &report.test_times, &report.query_times);
    auto end = std::chrono::high_resolution_clock::now();

    std::move(results.begin(), results.end(), std::inserter(report.results, report.results.begin()));
    report.total_elapsed_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    return report;
}

void format_report(QueryReport serial, QueryReport random, QueryReport semantic) {
    std::cout << "                    |     Serial     |     Random Clusters     |     Semantic Clusters     \n";
    std::cout << "Total results:      " << std::setw(10) << serial.result_count() << std::setw(10) << random.result_count() << std::setw(10) << semantic.result_count() << '\n';
    std::cout << "Skipped Clusters:   " << std::setw(10) << serial.skipped_clusters << std::setw(10) << random.skipped_clusters << std::setw(10) << semantic.skipped_clusters << '\n';
    std::cout << "Total elapsed time: " << std::setw(10) << serial.total_elapsed_time.count() << "us " << std::setw(10) << random.total_elapsed_time.count() << "us " << std::setw(10) << semantic.total_elapsed_time.count() << "us " << '\n';
    std::cout << "Average test time:  " << std::setw(10) << ' ' << std::setw(10) << random.average_test_time() << std::setw(10) << semantic.average_test_time() << '\n';
    std::cout << "Average query time: " << std::setw(10) << ' ' << std::setw(10) << random.average_query_time() << std::setw(10) << semantic.average_query_time() << '\n';;
}

int main(int argc, char **argv) {

    // spdlog::set_level(spdlog::level::debug);

    ProgramArgs args(argc, argv);

    if (args.showHelp) {
        std::cout << "Help!" << std::endl;
        return 0;
    }

    // Take semantic clusters
    auto semantic_clusters = rereuse::db::read_semantic_clusters(args.cluster_file);
    spdlog::info("Loaded {} semantic clusters", semantic_clusters.size());

    // Duplicate all the patterns
    auto all_patterns = unpack_patterns(semantic_clusters);
    spdlog::debug("Unpacked {} patterns from all clusters", all_patterns.size());

    // Create randomized clusters
    auto randomized_clusters = randomize_clusters(all_patterns, 100);
    spdlog::debug("Create {} new randomized clusters", randomized_clusters.size());

    // See how each performs
    std::unordered_set<std::string> positive_examples = {"cmsale@purdue.edu", "chucks.8090@gmail.com"};
    std::unordered_set<std::string> negative_examples = {"cmsale", "cmsale@", "cmsale@purdue"};

    std::unique_ptr<rereuse::query::BaseRegexQuery> serial_query = std::make_unique<rereuse::query::MatchQuery>(positive_examples, negative_examples);
    std::unique_ptr<rereuse::query::BaseClusterQuery> cluster_query = std::make_unique<rereuse::query::ClusterMatchQuery>(positive_examples, negative_examples);

    rereuse::db::RegexClusterRepository semantic_repo;
    for (auto &cluster : semantic_clusters) {
        semantic_repo.add_cluster(std::move(cluster));
    }

    rereuse::db::RegexClusterRepository random_repo;
    for (auto &cluster : randomized_clusters) {
        random_repo.add_cluster(std::move(cluster));
    }

    spdlog::info("Measuring serial report...");
    QueryReport serial_report = measure_no_clusters(all_patterns, serial_query);
    spdlog::info("Measuring random report...");
    QueryReport random_report = measure_clusters(random_repo, cluster_query);
    spdlog::info("Measuring cluster report...");
    QueryReport semantic_report = measure_clusters(semantic_repo, cluster_query);
    spdlog::info("Measuring random report again...");
    random_report = measure_clusters(random_repo, cluster_query);
    spdlog::info("Measuring cluster report again...");
    semantic_report = measure_clusters(semantic_repo, cluster_query);

    format_report(serial_report, random_report, semantic_report);

    return 0;
}
