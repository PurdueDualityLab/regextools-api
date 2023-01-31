//
// Created by charlie on 1/25/23.
//

#ifndef REGEXTOOLS_SIMILARITY_TABLE_H
#define REGEXTOOLS_SIMILARITY_TABLE_H


#include <vector>
#include "regex_entity.h"

class SimilarityTable {
public:
    explicit SimilarityTable(std::vector<RegexEntity> entities);

    /**
     * Compute the similarity matrix with parallelism
     * @param workers Number of workers to use
     */
    void parallel_compute(unsigned int workers);

    /**
     * Convert the matrix to a similarity graph
     */
    void to_similarity_graph();

    /**
     * Output the ABC representation of the similarity graph
     * @param os Stream to write the output to
     * @return os
     */
    std::ostream &output_abc(std::ostream &os) const;

private:
    /**
     * Computes the scores for a single
     * @param to_compare_against The regex for the given row
     * @return Scores for the row for to_compare_against
     */
    std::vector<double> compute_row(const RegexEntity &to_compare_against) const;

    std::vector<RegexEntity> entities;
    std::vector<std::vector<double>> scores;
};


#endif //REGEXTOOLS_SIMILARITY_TABLE_H
