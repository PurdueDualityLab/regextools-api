//
// Created by charlie on 11/9/22.
//

#include "memory_report.h"

#include <malloc.h>

rereuse::util::CSV MemoryReport::create_report() const {
    rereuse::util::CSV memory_report;
    auto row = memory_report.push_row(this->key);
    auto pre_db_col = memory_report.push_col("Pre Database (b)");
    auto after_db_col = memory_report.push_col("Post Database (b)");
    auto after_query_col = memory_report.push_col("After Query (b)");
    auto db_size_col = memory_report.push_col("Database Size (mb)");
    auto query_size_col = memory_report.push_col("Query Size (mb)");

    memory_report.cell(row, pre_db_col) << this->pre_db;
    memory_report.cell(row, after_db_col) << this->post_db;
    memory_report.cell(row, after_query_col) << this->after_query;
    memory_report.cell(row, db_size_col) << (this->post_db - this->pre_db) / 1'000'000;
    memory_report.cell(row, query_size_col) << (this->after_query - this->post_db) / 1'000'000;

    return memory_report;
}

std::size_t get_total_allocated_space() {
    return mallinfo2().uordblks;
}
