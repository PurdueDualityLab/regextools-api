//
// Created by charlie on 10/18/22.
//

#include "csv.h"

#include <algorithm>

std::size_t rereuse::util::CSV::push_row(std::string header) {

    // Add the row header
    this->row_headers.push_back(std::move(header));
    // Push a new row with the column count
    auto idx = this->row_headers.size();
    this->data.emplace_back(this->column_headers.size());

    return idx;
}

std::size_t rereuse::util::CSV::push_col(std::string header) {
    auto idx = this->column_headers.size();
    this->column_headers.push_back(std::move(header));

    for (auto &row : this->data) {
        row.emplace_back();
    }

    return idx;
}

std::ostringstream &rereuse::util::CSV::cell(std::size_t row, std::size_t col) {
    // Check bounds
    if (row > this->row_headers.size() || col > this->column_headers.size()) {
        throw std::out_of_range("Cell is out of range");
    }

    // Values are in bounds. return a reference
    return this->data[row][col];
}

std::string rereuse::util::CSV::cell(std::size_t row, std::size_t col) const {
    // Check bounds
    if (row > this->row_headers.size() || col > this->column_headers.size()) {
        throw std::out_of_range("Cell is out of range");
    }

    // Values are in bounds. return a reference
    return this->data[row][col].str();
}

bool rereuse::util::CSV::has_non_empty_row_headers() const {
    return std::any_of(this->row_headers.cbegin(), this->row_headers.cend(),
                [] (const std::string &hdr) { return !hdr.empty(); });
}

bool rereuse::util::CSV::has_non_empty_col_headers() const {
    return std::any_of(this->column_headers.cbegin(), this->column_headers.cend(),
                       [] (const std::string &hdr) { return !hdr.empty(); });
}

std::ostream &rereuse::util::operator<<(std::ostream &os, const rereuse::util::CSV &csv) {
    if (csv.has_non_empty_col_headers()) {
        if (csv.has_non_empty_row_headers())
            os << ',';

        for (const auto &col_header : csv.column_headers) {
            os << col_header << ',';
        }
        os << '\n';
    }

    for (std::size_t row = 0; row < csv.row_headers.size(); row++) {
        if (csv.has_non_empty_row_headers()) {
            os << csv.row_headers[row] << ',';
        }

        for (std::size_t col = 0; col < csv.column_headers.size(); col++) {
            os << csv.cell(row, col) << ',';
        }
        os << '\n';
    }

    return os;
}
