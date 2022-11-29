//
// Created by charlie on 11/18/22.
//

#ifndef REGEXTOOLS_QUERY_REPORT_H
#define REGEXTOOLS_QUERY_REPORT_H

#include <chrono>
#include <unordered_set>
#include <string>
#include <utility>

namespace rereuse::query {

    struct QueryResult {
        QueryResult(std::unordered_set<std::string> results, unsigned long skippedClusters,
                    const std::chrono::microseconds &medianTestFailTime,
                    const std::chrono::microseconds &medianTestPassTime,
                    const std::chrono::microseconds &medianDrillTime,
                    double averageMatchVectorSize)
        : results(std::move(results))
        , skipped_clusters(skippedClusters)
        , median_test_fail_time(medianTestFailTime)
        , median_test_pass_time(medianTestPassTime)
        , median_drill_time(medianDrillTime)
        , average_match_vector_size(averageMatchVectorSize) {}

        std::unordered_set<std::string> results;
        unsigned long skipped_clusters;
        std::chrono::microseconds median_test_fail_time;
        std::chrono::microseconds median_test_pass_time;
        std::chrono::microseconds median_drill_time;
        double average_match_vector_size;
    };

    struct QueryReport {
        QueryReport(const QueryResult &result)
        : results(result.results)
        , positive_examples_count(0)
        , negative_examples_count(0)
        , skipped_clusters(result.skipped_clusters)
        , total_elapsed_time()
        , median_test_pass_time(result.median_test_pass_time)
        , median_test_fail_time(result.median_test_fail_time)
        , median_drill_time(result.median_drill_time)
        , average_vec_size(result.average_match_vector_size)
        {}

        QueryReport(QueryResult &&result)
                : results(std::move(result.results))
                , positive_examples_count(0)
                , negative_examples_count(0)
                , skipped_clusters(result.skipped_clusters)
                , total_elapsed_time()
                , median_test_pass_time(result.median_test_pass_time)
                , median_test_fail_time(result.median_test_fail_time)
                , median_drill_time(result.median_drill_time)
                , average_vec_size(result.average_match_vector_size)
        {}

        QueryReport()
                : positive_examples_count(0)
                , negative_examples_count(0)
                , skipped_clusters(0)
                , total_elapsed_time()
                , average_vec_size(0)
        {}

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
    };
}

#endif //REGEXTOOLS_QUERY_REPORT_H
