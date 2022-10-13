//
// Created by charlie on 10/10/22.
//

#ifndef REGEXTOOLS_REPORT_H
#define REGEXTOOLS_REPORT_H

#include <string>
#include <chrono>
#include <unordered_set>
#include <unordered_map>
#include <optional>

class Report {
public:
    /**
     * Represents the most precise duration used by this report
     */
    using Duration = std::chrono::microseconds;

    Report() = default;

    explicit Report(std::string title)
    : title(std::move(title))
    {}

    void add_category(const std::string &label, Duration duration) {
        this->category_durations[label] = duration;
    }

    std::unordered_set<std::string> label_set() const;

    std::optional<Report::Duration> get_category_duration(const std::string &label) const;

    friend std::ostream &operator<<(std::ostream &os, const Report &report);

private:
    std::pair<unsigned long, unsigned int> longest_labels() const;

    std::string title;
    std::unordered_map<std::string, Duration> category_durations;
};

class AggregateReport {
public:
    explicit AggregateReport(std::string title)
    : title(std::move(title))
    {}

    void add_report(const std::string &reportTitle, Report report) {
        this->reports[reportTitle] = std::move(report);
    }

    friend std::ostream &operator<<(std::ostream &os, const AggregateReport &report);
private:
    std::unordered_set<std::string> all_duration_categories() const;

    std::string title;
    std::unordered_map<std::string, Report> reports;
};

#endif //REGEXTOOLS_REPORT_H
