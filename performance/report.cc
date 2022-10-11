//
// Created by charlie on 10/10/22.
//

#include <iomanip>
#include <algorithm>
#include "report.h"

std::ostream &operator<<(std::ostream &os, const Report &report) {
    const std::string category_header = "Category:";
    const std::string value_header = "Duration:";

    // Figure out what column widths to set for each item is
    auto [longest_key, longest_duration] = report.longest_labels();
    longest_key = std::max(longest_key, category_header.length());
    longest_duration = std::max(static_cast<unsigned long>(longest_duration), value_header.length());
    auto key_width = static_cast<int>(longest_key);
    auto val_width = static_cast<int>(longest_duration) + 2;

    os << report.title << '\n';
    os << std::setw(key_width) << category_header << std::setw(val_width) << value_header << '\n';
    for (const auto &[label, duration] : report.category_durations) {
        os << std::setw(key_width) << label << std::setw(val_width) << duration.count() << "us" << '\n';
    }
    os << std::flush;

    return os;
}

unsigned int digit_count(unsigned long tick_count) {
    unsigned int digits = 0;
    while (tick_count > 0) {
        tick_count /= 10;
        digits++;
    }

    return digits;
}

std::pair<unsigned long, unsigned int> Report::longest_labels() const {
    unsigned long longest_key = 0;
    unsigned int longest_digits = 0;
    for (const auto &[label, duration] : this->category_durations) {
        // Get the longest key
        longest_key = std::max(longest_key, label.length());
        // Get the longest label
        longest_digits = std::max(longest_digits, digit_count(duration.count()));
    }

    return {longest_key, longest_digits};
}

std::optional<Report::Duration> Report::get_category_duration(const std::string &label) const {
    try {
        const auto &duration = this->category_durations.at(label);
        return { duration };
    } catch (std::out_of_range &exe) {
        return {};
    }
}

std::unordered_set<std::string> Report::label_set() const {
    std::unordered_set<std::string> keys;
    for (const auto &[key, _] : this->category_durations) {
        keys.insert(key);
    }

    return keys;
}

std::ostream &operator<<(std::ostream &os, const AggregateReport &report) {

    os << report.title << '\n';



    return os;
}

std::unordered_set<std::string> AggregateReport::all_duration_categories() const {
    std::unordered_set<std::string> all_labels;
    for (const auto &[_, report] : this->reports) {
        auto labels = report.label_set();
        std::set_union(all_labels.cbegin(), all_labels.cend(),
                       labels.cbegin(), labels.cend(),
                       std::inserter(all_labels, all_labels.begin()));
    }

    return all_labels;
}
