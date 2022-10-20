//
// Created by charlie on 10/18/22.
//

#ifndef REGEXTOOLS_CSV_H
#define REGEXTOOLS_CSV_H

#include <vector>
#include <string>
#include <sstream>

namespace rereuse::util {

    class CSV {
    public:
        CSV() = default;

        std::size_t push_row(std::string header = "");
        std::size_t push_col(std::string header = "");

        std::ostringstream &cell(std::size_t row, std::size_t col);
        std::string cell(std::size_t row, std::size_t col) const;

        friend std::ostream &operator<<(std::ostream &os, const CSV &csv);

    protected:
        bool has_non_empty_row_headers() const;
        bool has_non_empty_col_headers() const;

        std::vector<std::string> row_headers;
        std::vector<std::string> column_headers;
        std::vector<std::vector<std::ostringstream>> data;
    };
} // util

#endif //REGEXTOOLS_CSV_H
