//
// Created by charlie on 10/12/22.
//

#include <array>
#include <string_view>
#include "query_report.h"
#include "librereuse/util/stats.h"

double QueryReport::average_test_time() const {
    if (this->test_times.empty())
        return 0;

    // TODO fix this stuff up
    if (this->test_times.size() == 1)
        return this->test_times[0].count();

    return rereuse::util::mean_duration(this->test_times.cbegin(), this->test_times.cend()).count();
}

double QueryReport::average_query_time() const {
    if (this->query_times.empty())
        return 0;

    // TODO fix this stuff up
    if (this->query_times.size() == 1)
        return this->query_times[0].count();

    return rereuse::util::mean_duration(this->query_times.cbegin(), this->query_times.cend()).count();
}

BenchmarkReport::BenchmarkReport() {

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
    this->csv.cell(BenchmarkReportLabels::key_idx("Average Positive Vector Size"sv), col_idx) << report.average_vec_size;
#if 0
    this->csv.cell(BenchmarkReportLabels::key_idx("Total Elapsed Time (ms)"sv), col_idx) << report.total_elapsed_time.count();
    this->csv.cell(BenchmarkReportLabels::key_idx("Median Test Time (us)"sv), col_idx) << report.median_test_time().count();
    this->csv.cell(BenchmarkReportLabels::key_idx("Median Drill Time (us)"sv), col_idx) << report.median_query_time().count();
#else
    this->csv.cell(BenchmarkReportLabels::key_idx("Total Elapsed Time (ms)"sv), col_idx) << to_ms_double(report.total_elapsed_time);
    this->csv.cell(BenchmarkReportLabels::key_idx("Median Test Time (ms)"sv), col_idx) << to_ms_double(report.median_test_time());
    this->csv.cell(BenchmarkReportLabels::key_idx("Median Drill Time (ms)"sv), col_idx) << to_ms_double(report.median_query_time());
#endif
}

std::ostream &operator<<(std::ostream &os, const BenchmarkReport &report) {
    os << report.csv;
    return os;
}

QueryReport median_query_report(const std::vector<QueryReport> &reports) {
    std::vector<std::chrono::milliseconds> total_elapsed_times;
    std::vector<std::chrono::microseconds> average_test_time;
    std::vector<std::chrono::microseconds> average_drill_time;
    for (const auto &report : reports) {
        total_elapsed_times.push_back(report.total_elapsed_time);
        std::copy(report.test_times.cbegin(), report.test_times.cend(), std::back_inserter(average_test_time));
        std::copy(report.query_times.cbegin(), report.query_times.cend(), std::back_inserter(average_drill_time));
    }

    QueryReport median_report(reports[0]);
    median_report.test_times.clear();
    median_report.query_times.clear();
    auto median_elapsed = rereuse::util::median_duration(total_elapsed_times.cbegin(), total_elapsed_times.cend());

    median_report.total_elapsed_time = median_elapsed;
    std::move(average_test_time.cbegin(), average_test_time.cend(), std::back_inserter(median_report.test_times));
    std::move(average_drill_time.cbegin(), average_drill_time.cend(), std::back_inserter(median_report.query_times));

    return median_report;
}
