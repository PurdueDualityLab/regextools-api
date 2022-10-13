//
// Created by charlie on 10/12/22.
//

#include "query_report.h"
#include "librereuse/util/stats.h"

double QueryReport::average_test_time() const {
    std::vector<std::chrono::microseconds::rep> microseconds;
    std::transform(this->test_times.cbegin(), this->test_times.cend(), std::back_inserter(microseconds),
                   [](const auto &item) { return item.count(); });

    return rereuse::util::mean(microseconds.cbegin(), microseconds.cend());
}

double QueryReport::average_query_time() const {
    std::vector<std::chrono::microseconds::rep> microseconds;
    std::transform(this->query_times.cbegin(), this->query_times.cend(), std::back_inserter(microseconds),
                   [](const auto &item) { return item.count(); });

    return rereuse::util::mean(microseconds.cbegin(), microseconds.cend());
}
