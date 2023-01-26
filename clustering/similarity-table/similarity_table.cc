//
// Created by charlie on 1/25/23.
//

#include "similarity_table.h"

#include <iostream>
#include <string>
#include <sstream>
#include <thread>
#include <spdlog/spdlog.h>
#include "ThreadPool.h"

static std::string convert_thread_id(std::thread::id id) {
    std::stringstream ss;
    ss << id;
    return ss.str();
}

SimilarityTable::SimilarityTable(std::vector<RegexEntity> entities)
: entities(std::move(entities)) {
    // Create a zero square matrix
    scores = std::vector<std::vector<double>>(this->entities.size(), std::vector<double> (this->entities.size(), 0));
}

std::vector<double> SimilarityTable::compute_row(const RegexEntity &to_compare_against) const {
    const auto &regex = to_compare_against.get_regex();
    auto thread_id = convert_thread_id(std::this_thread::get_id());
    std::vector<double> row_scores;
    for (const auto &entity : this->entities) {
        unsigned long hits = 0;
        std::this_thread::get_id();
        spdlog::info("{}: comparing /{}/ to /{}/...", thread_id, regex->pattern(), entity.get_regex()->pattern());
        for (const auto &str : entity.get_strings()) {
            if (re2::RE2::PartialMatch(str, *regex)) {
                hits++;
            }
        }
        row_scores.push_back(static_cast<double>(hits) / static_cast<double>(entity.get_strings().size()));
    }

    return row_scores;
}

void SimilarityTable::parallel_compute(unsigned int workers) {
    ThreadPool thread_pool(workers);
    std::vector<std::future<std::pair<std::size_t, std::vector<double>>>> row_tasks;
    for (std::size_t row_idx = 0; row_idx < entities.size(); row_idx++) {
        const auto &entity = this->entities[row_idx];
        auto task = thread_pool.enqueue([this, &entity, row_idx] {
            auto row_scores = this->compute_row(entity);
            return std::make_pair(row_idx, row_scores);
        });
        row_tasks.push_back(std::move(task));
    }

    // Swap out the scores for each row
    for (auto &task : row_tasks) {
        auto [row_idx, row_scores] = task.get();
        this->scores[row_idx].swap(row_scores);
    }
}

void SimilarityTable::to_similarity_graph() {
    std::vector<std::vector<double>> half_matrix(this->entities.size());
    for (int row = 0; row < this->scores.size(); row++) {
        half_matrix[row] = std::vector<double>(row + 1);
        for (int col = 0; col <= row; col++) {
            double average = (this->scores[row][col] + this->scores[col][row]) / 2.0;
            half_matrix[row][col] = average;
        }
    }

    this->scores = std::move(half_matrix);
}

std::ostream &SimilarityTable::output_abc(std::ostream &os) const {
    for (int row_idx = 0; row_idx < scores.size(); row_idx++) {
        auto row = this->scores[row_idx];
        for (int col_idx = 0; col_idx < row.size(); col_idx++) {
            double score = row[col_idx];
            if (score > 0) {
                os << entities[row_idx].get_regex()->pattern() << '\t' << entities[col_idx].get_regex()->pattern() << '\t' << score << std::endl;
            }
        }
    }

    return os;
}


