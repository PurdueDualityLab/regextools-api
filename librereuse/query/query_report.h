//
// Created by charlie on 11/18/22.
//

#ifndef REGEXTOOLS_QUERY_REPORT_H
#define REGEXTOOLS_QUERY_REPORT_H

#include <chrono>
#include <unordered_set>
#include <string>
#include <utility>
#include <unordered_map>

namespace rereuse::query {

    struct QueryResult {
        QueryResult(std::unordered_set<std::string> results, unsigned long skippedClusters,
                    const std::chrono::microseconds &medianTestFailTime,
                    const std::chrono::microseconds &medianTestPassTime,
                    const std::chrono::microseconds &medianDrillTime,
                    double averageMatchVectorSize);

        std::unordered_set<std::string> results;
        unsigned long skipped_clusters;
        std::chrono::microseconds median_test_fail_time;
        std::chrono::microseconds median_test_pass_time;
        std::chrono::microseconds median_drill_time;
        double average_match_vector_size;
    };

    struct QueryReport {
        QueryReport(const QueryResult &result);

        QueryReport(QueryResult &&result);

        explicit QueryReport();

        unsigned long result_count() const { return results.size(); }

        std::unordered_set<std::string> results;
        std::size_t positive_examples_count;
        std::size_t negative_examples_count;
        unsigned long skipped_clusters;
        std::chrono::milliseconds total_elapsed_time;
        std::chrono::microseconds median_test_pass_time{};
        std::chrono::microseconds median_test_fail_time{};
        std::chrono::microseconds median_drill_time{};
        double average_vec_size;
        std::unordered_map<unsigned long, unsigned long> result_cluster_info;
    };
}

#endif //REGEXTOOLS_QUERY_REPORT_H
