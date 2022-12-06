//
// Created by charlie on 12/3/22.
//

#ifndef REGEXTOOLS_SIMILARITY_TABLE_H
#define REGEXTOOLS_SIMILARITY_TABLE_H

#include <vector>
#include <string>
#include <thread>
#include <memory>
// #include <threadpool/ThreadPool.h>
#include "misc/ThreadPool.h"

#define CONCURRENT_COMPUTE 1

#if CONCURRENT_COMPUTE
template <typename ScorerTp>
std::pair<std::size_t, std::vector<double>> score_row(std::size_t row_idx, std::shared_ptr<ScorerTp> base, std::vector<std::shared_ptr<ScorerTp>> row) {
    spdlog::info("{}: Starting scoring row {}...", std::this_thread::get_id(), row_idx);
    std::vector<double> scores(row.size());
    std::size_t idx = 0;
    for (const auto &entry : row) {
        if (entry->get_id() == base->get_id()) {
            scores[idx++] = 1;
            continue;
        }

        scores[idx++] = base->score(*entry);
        spdlog::debug("{}: scored ({},{})", std::this_thread::get_id(), row_idx, idx);
    }

    spdlog::info("{}: Scored row {}.", std::this_thread::get_id(), row_idx);

    return {row_idx, scores};
}
#endif

template <typename ScorerTp>
class SimilarityTable {
public:
    explicit SimilarityTable(std::vector<std::shared_ptr<ScorerTp>> scorers) {
        // Set up the score table
        this->scorers = std::move(scorers);

        // Fill out the scores table
        for (unsigned long idx = 0; idx < this->scorers.size(); idx++) {
            std::vector<double> row(this->scorers.size());
            this->scores.push_back(std::move(row));
        }
    }

    // Actually compute the things
#if !CONCURRENT_COMPUTE
    void compute() {
        for (unsigned long row = 0; row < scorers.size(); row++) {
            auto base_scorer = this->scorers[row];
            spdlog::info("Starting scoring for {}...", row);
            for (unsigned long col = 0; col < scorers.size(); col++) {
                // Diagonal matrix
                if (row == col) {
                    this->scores[row][col] = 1;
                    continue;
                }

                // Compute the score between the two
                auto other_scorer = this->scorers[col];
                this->scores[row][col] = base_scorer->score(*other_scorer);
                spdlog::debug("Scored ({},{})", row, col);
            }
            spdlog::info("Finished scoring for {}", row);
        }
    }
#else
    void compute() {
        auto score_lambda = [](std::size_t row_idx, std::shared_ptr<ScorerTp> base, std::vector<std::shared_ptr<ScorerTp>> row) -> std::pair<std::size_t, std::vector<double>> {
            std::stringstream ss;
            ss << std::this_thread::get_id();
            auto thread_id = ss.str();
            spdlog::info("{}: Starting scoring row {}...", thread_id, row_idx);
            std::vector<double> row_scores(row.size());
            std::size_t idx = 0;
            for (const auto &entry : row) {
                if (entry->get_id() == base->get_id()) {
                    row_scores[idx++] = 1;
                    continue;
                }

                row_scores[idx++] = base->score(*entry);
                spdlog::debug("{}: scored ({},{})", thread_id, row_idx, idx);
            }

            spdlog::info("{}: Scored row {}.", thread_id, row_idx);

            return {row_idx, row_scores};
        };

        ThreadPool tp(8);
        std::vector<std::future<std::pair<std::size_t, std::vector<double>>>> tasks;
        std::size_t idx = 0;
        for (const auto &scorer : this->scorers) {
            auto task = tp.template enqueue(score_lambda, idx++, scorer, this->scorers);
            tasks.push_back(std::move(task));
        }

        // Collect each task and place it in the scores array
        for (auto &task : tasks) {
            auto [index, scored_row] = task.get();
            this->scores[index].swap(scored_row);
        }
    }
#endif

    void to_similarity_graph() {
        std::vector<std::vector<double>> half_matrix(this->scorers.size());
        for (int row = 0; row < this->scores.size(); row++) {
            half_matrix[row] = std::vector<double>(row + 1);
            for (int col = 0; col <= row; col++) {
                double average = (this->scores[row][col] + this->scores[col][row]) / 2.0;
                // this->scores[row][col] = average;
                half_matrix[row][col] = average;
            }
        }

        this->scores = std::move(half_matrix);
    }

    std::string to_abc(const std::string &abc_graph_output) {
        std::ofstream abc_fstream(abc_graph_output);

        for (int row_idx = 0; row_idx < scores.size(); row_idx++) {
            auto row = this->scores[row_idx];
            for (int col_idx = 0; col_idx < row.size(); col_idx++) {
                double score = row[col_idx];
                if (score > 0) {
                    abc_fstream << this->scorers[row_idx]->get_id() << '\t' << this->scorers[col_idx]->get_id() << '\t' << score << std::endl;
                }
            }
        }

        abc_fstream.close();

        return abc_graph_output;
    }

    void SimilarityTable::top_k_edges(unsigned int edges) {
        for (auto &row : this->scores) {
            // If there aren't enough edges, continue
            if (row.size() <= edges)
                continue;

            // Make an enumeration of all the outgoing edges
            std::vector<std::pair<unsigned int, double>> indexed_scores;
            unsigned int index = 0;
            std::transform(row.cbegin(), row.cend(), std::back_inserter(indexed_scores),
                           [&index](double score) -> std::pair<unsigned int, double> { return { index++, score }; });

            auto score_comparer = [](const std::pair<unsigned int, double> &left, const std::pair<unsigned int, double> &right) {
                return left.second > right.second;
            };
            std::priority_queue<
                    std::pair<unsigned int, double>,
                    std::vector<std::pair<unsigned int, double>>,
                    decltype(score_comparer)
            > ordered_indexed_scores(score_comparer, indexed_scores);

            // Remove small edges until this row only has edges number of edges
            while (ordered_indexed_scores.size() > edges) {
                auto top = ordered_indexed_scores.top();
                ordered_indexed_scores.pop();
                auto idx = top.first;
                row[idx] = 0;
            }
        }
    }

private:
    std::vector<std::vector<double>> scores;
    std::vector<std::shared_ptr<ScorerTp>> scorers;
};

#endif //REGEXTOOLS_SIMILARITY_TABLE_H
