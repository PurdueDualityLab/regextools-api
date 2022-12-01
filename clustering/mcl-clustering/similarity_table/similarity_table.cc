//
// Created by charlie on 2/10/22.
//

#include "similarity_table.h"

#include <queue>

SimilarityTable::SimilarityTable(const std::unordered_map<unsigned long, std::string> &patterns, unsigned int workers,
                                 std::function<std::shared_ptr<BaseSimilarityScorer>(unsigned long,std::string)> scorer_constructor,
                                 bool computeMatrix)
        : has_temp_abc_file(false) {

    ThreadPool thread_pool(workers);
    std::vector<std::future<std::shared_ptr<BaseSimilarityScorer>>> scorer_futures;
    for (const auto &[id, pattern] : patterns) {
        auto new_score_future = thread_pool.enqueue(scorer_constructor, id, pattern);
        scorer_futures.push_back(std::move(new_score_future));
    }

    // take all the futures and put them in scorers
    for (auto &&future : scorer_futures) {
        future.wait();
        auto new_scorer = std::move(future.get());
        if (new_scorer)
            this->scorers.push_back(std::move(new_scorer));
    }
    std::cout << "Created " << this->scorers.size() << " scorers" << std::endl;

    if (computeMatrix) {
        // Now compute the similarity matrix
        this->scores = std::vector<std::vector<double>>(this->scorers.size());
        for (size_t row = 0; row < this->scorers.size(); row++) {
            this->scores[row] = std::vector<double>(this->scorers.size());
        }

        // Make a bunch of tasks to compute the similarity matrix
        std::vector<std::future<std::tuple<size_t, size_t, double>>> scoring_tasks;
        for (size_t row = 0; row < this->scorers.size(); row++) {
            auto row_scorer = this->scorers[row];
            for (size_t col = 0; col < this->scorers.size(); col++) {
                auto col_scorer = this->scorers[col];
                auto task = thread_pool.enqueue([row, col, row_scorer, col_scorer] {
                    // Compute the score
                    double score = row_scorer->score(col_scorer);
                    std::cout << "Scored (" << row << ',' << col << ')' << std::endl;
                    return std::tuple<size_t, size_t, double>{row, col, score};
                });
                scoring_tasks.push_back(std::move(task));
            }
        }

        // Get all the scores
        for (auto &future: scoring_tasks) {
            future.wait();
            auto[row, col, score] = future.get();
            this->scores[row][col] = score;
        }
    }
}

SimilarityTable::~SimilarityTable() {
    // Try deleting the abc file
    if (has_temp_abc_file)
        remove(this->abc_file.c_str());
}

void SimilarityTable::to_similarity_graph() {
    std::vector<std::vector<double>> half_matrix(this->scorers.size());
    for (int row = 0; row < this->scores.size(); row++) {
        half_matrix[row] = std::move(std::vector<double>(row + 1));
        for (int col = 0; col <= row; col++) {
            double average = (this->scores[row][col] + this->scores[col][row]) / 2.0;
            // this->scores[row][col] = average;
            half_matrix[row][col] = average;
        }
    }

    this->scores = std::move(half_matrix);
}

std::string SimilarityTable::to_abc() {
    this->abc_file = "/tmp/abc_graph_" + std::to_string(random()) + ".abc";
    this->has_temp_abc_file = true;

    return this->to_abc(this->abc_file);
}

std::string SimilarityTable::to_abc(const std::string &abc_graph_output) {
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

void SimilarityTable::prune(double threshold) {
    for (auto &row : this->scores) {
        for (auto &score : row) {
            if (score < threshold)
                score = 0.0;
        }
    }
}

std::optional<std::string> SimilarityTable::get_pattern(unsigned long id) const {
    for (const auto &scorer : this->scorers) {
        if (scorer->get_id() == id) {
            return { scorer->get_pattern() };
        }
    }

    return {};
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
