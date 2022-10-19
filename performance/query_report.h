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

struct QueryReport {
    QueryReport()
    : skipped_clusters(0)
    , total_elapsed_time()
    , average_vec_size(0)
    {}

    double average_test_time() const;
    double average_query_time() const;

    unsigned long result_count() const { return results.size(); }

    std::unordered_set<std::string> results;
    unsigned long skipped_clusters;
    std::chrono::microseconds total_elapsed_time;
    std::vector<std::chrono::microseconds> test_times;
    std::vector<std::chrono::microseconds> query_times;
    double average_vec_size;
};

#endif //REGEXTOOLS_QUERY_REPORT_H
