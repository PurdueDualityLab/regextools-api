//
// Created by charlie on 10/12/22.
//

#include <array>
#include <string_view>
#include "query_report.h"
#include "librereuse/util/stats.h"


BenchmarkReport::BenchmarkReport()
: cluster_count(0) {
    // Create a row for each category
    for (const auto &category : BenchmarkReportLabels::categories) {
        this->csv.push_row(std::string(category.cbegin(), category.cend()));
    }
}

BenchmarkReport::BenchmarkReport(std::size_t cluster_count)
: cluster_count(cluster_count) {

    // Create a row for each category
    for (const auto &category : BenchmarkReportLabels::categories) {
        this->csv.push_row(std::string(category.cbegin(), category.cend()));
    }
}

template <class DurationTp>
double to_ms_double(const DurationTp &duration) {
    std::chrono::duration<double, std::milli> ms {};
    ms = std::chrono::duration_cast<decltype(ms)>(duration);

    return ms.count();
}

void BenchmarkReport::add_query_report(std::string label, const QueryReport &report) {
    auto col_idx = this->csv.push_col(std::move(label));
    this->csv.cell(BenchmarkReportLabels::key_idx("Positive Examples"sv), col_idx) << report.positive_examples_count;
    this->csv.cell(BenchmarkReportLabels::key_idx("Negative Examples"sv), col_idx) << report.negative_examples_count;
    this->csv.cell(BenchmarkReportLabels::key_idx("Results"sv), col_idx) << report.result_count();
    this->csv.cell(BenchmarkReportLabels::key_idx("Skipped Clusters"sv), col_idx) << report.skipped_clusters;
    this->csv.cell(BenchmarkReportLabels::key_idx("Skipped Cluster Percentage"sv), col_idx) << this->skipped_cluster_percentage(report.skipped_clusters);
    this->csv.cell(BenchmarkReportLabels::key_idx("Average Positive Vector Size"sv), col_idx) << report.average_vec_size;
#if 0
    this->csv.cell(BenchmarkReportLabels::key_idx("Total Elapsed Time (ms)"sv), col_idx) << report.total_elapsed_time.count();
    this->csv.cell(BenchmarkReportLabels::key_idx("Median Test Time (us)"sv), col_idx) << report.median_test_time().count();
    this->csv.cell(BenchmarkReportLabels::key_idx("Median Drill Time (us)"sv), col_idx) << report.median_query_time().count();
#else
    this->csv.cell(BenchmarkReportLabels::key_idx("Total Elapsed Time (ms)"sv), col_idx) << to_ms_double(report.total_elapsed_time);
    this->csv.cell(BenchmarkReportLabels::key_idx("Median Test Hit Time (ms)"sv), col_idx) << to_ms_double(report.median_test_pass_time);
    this->csv.cell(BenchmarkReportLabels::key_idx("Median Test Fail Time (ms)"sv), col_idx) << to_ms_double(report.median_test_fail_time);
    this->csv.cell(BenchmarkReportLabels::key_idx("Median Drill Time (ms)"sv), col_idx) << to_ms_double(report.median_drill_time);
#endif
}

std::ostream &operator<<(std::ostream &os, const BenchmarkReport &report) {
    os << report.csv;
    return os;
}

double BenchmarkReport::skipped_cluster_percentage(std::size_t skipped) const noexcept {
    if (this->cluster_count > 0) {
        return skipped / static_cast<double>(this->cluster_count);
    } else {
        return 0;
    }
}

QueryReport median_query_report(const std::vector<QueryReport> &reports) {
    std::vector<std::chrono::milliseconds> total_elapsed_times;
    std::vector<std::chrono::microseconds> test_pass_times;
    std::vector<std::chrono::microseconds> test_fail_times;
    std::vector<std::chrono::microseconds> drill_times;
    for (const auto &report : reports) {
        total_elapsed_times.push_back(report.total_elapsed_time);
        test_pass_times.push_back(report.median_test_pass_time);
        test_fail_times.push_back(report.median_test_fail_time);
        drill_times.push_back(report.median_drill_time);
    }

    QueryReport median_report(reports[0]);

    median_report.total_elapsed_time = rereuse::util::median_duration(total_elapsed_times.cbegin(), total_elapsed_times.cend());
    median_report.median_test_pass_time = rereuse::util::median_duration(test_pass_times.cbegin(),
                                                                         test_pass_times.cend());
    median_report.median_test_fail_time = rereuse::util::median_duration(test_fail_times.cbegin(),
                                                                         test_fail_times.cend());
    median_report.median_drill_time = rereuse::util::median_duration(drill_times.cbegin(), drill_times.cend());

    return median_report;
}
