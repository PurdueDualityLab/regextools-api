//
// Created by charlie on 11/9/22.
//

#ifndef REGEXTOOLS_MEMORY_REPORT_H
#define REGEXTOOLS_MEMORY_REPORT_H

#include <cstddef>

#include "librereuse/util/csv.h"

struct MemoryReport {
    std::size_t pre_db;
    std::size_t post_db;
    std::size_t after_query;

    std::string key;

    MemoryReport(std::string key)
    : pre_db(0)
    , post_db(0)
    , after_query(0)
    , key(std::move(key))
    {}

    rereuse::util::CSV create_report() const;
};

#endif //REGEXTOOLS_MEMORY_REPORT_H
