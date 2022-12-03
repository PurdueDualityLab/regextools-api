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



private:
    std::vector<std::vector<double>> scores;
    std::vector<ScorerTp> scorers;
};


#endif //REGEXTOOLS_SIMILARITY_TABLE_H
