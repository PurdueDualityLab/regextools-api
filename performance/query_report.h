//
// Created by charlie on 10/12/22.
//

#ifndef REGEXTOOLS_QUERY_REPORT_H
#define REGEXTOOLS_QUERY_REPORT_H

#include <string>
#include <chrono>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <string_view>
#include <algorithm>
#include "librereuse/util/csv.h"
#include "librereuse/util/stats.h"

struct QueryReport {
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

using namespace std::string_view_literals;
struct BenchmarkReportLabels {

    static constexpr std::array<std::string_view, 10> categories = {
            "Positive Examples"sv,
            "Negative Examples"sv,
            "Results"sv,
            "Skipped Clusters"sv,
            "Skipped Cluster Percentage"sv,
            "Average Positive Vector Size"sv,
            "Total Elapsed Time (ms)"sv,
            "Median Test Hit Time (ms)"sv,
            "Median Test Fail Time (ms)"sv,
            "Median Drill Time (ms)"sv,
    };

    static constexpr long key_idx(std::string_view key) {
        auto idx = 0;
        for (const auto &category : categories) {
            if (category == key) {
                return idx;
            } else {
                idx++;
            }
        }

        return -1;
    }
};

class BenchmarkReport {
public:
    BenchmarkReport();
    explicit BenchmarkReport(std::size_t cluster_count);

    void add_query_report(std::string label, const QueryReport &report);

    double skipped_cluster_percentage(std::size_t skipped) const noexcept;

    friend std::ostream &operator<<(std::ostream &os, const BenchmarkReport &report);

private:
    std::size_t cluster_count;
    rereuse::util::CSV csv;
};

QueryReport median_query_report(const std::vector<QueryReport> &reports);

#endif //REGEXTOOLS_QUERY_REPORT_H
