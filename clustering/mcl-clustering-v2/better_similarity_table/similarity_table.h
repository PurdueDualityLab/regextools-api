//
// Created by charlie on 12/3/22.
//

#ifndef REGEXTOOLS_SIMILARITY_TABLE_H
#define REGEXTOOLS_SIMILARITY_TABLE_H

#include <vector>
#include <string>

template <typename ScorerTp>
class SimilarityTable {
public:
    explicit SimilarityTable(std::vector<ScorerTp> scorers) {
        // Set up the score table
        this->scorers = std::move(scorers);

        // Fill out the scorer table
        for (unsigned long idx = 0; idx < scorers.size(); idx++) {
            this->scores.template emplace_back(scorers.size(), 0);
        }
    }

    // Actually compute the things
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
                this->scores[row][col] = base_scorer.score(other_scorer);
                spdlog::debug("Scored ({},{})", row, col);
            }
            spdlog::info("Finished scoring for {}", row);
        }
    }

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
                    abc_fstream << this->scorers[row_idx].get_id() << '\t' << this->scorers[col_idx].get_id() << '\t' << score << std::endl;
                }
            }
        }

        abc_fstream.close();

        return abc_graph_output;
    }

private:
    std::vector<std::vector<double>> scores;
    std::vector<ScorerTp> scorers;
};


#endif //REGEXTOOLS_SIMILARITY_TABLE_H
