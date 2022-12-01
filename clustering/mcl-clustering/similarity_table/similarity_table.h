//
// Created by charlie on 2/10/22.
//

#ifndef _SIMILARITY_TABLE_H
#define _SIMILARITY_TABLE_H

#include <iostream>
#include <vector>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <optional>
#include "base_similarity_scorer.h"
#include "../ThreadPool.h"
#include "rex_similarity_scorer.h"

class SimilarityTable {
public:
    explicit SimilarityTable(const std::unordered_map<unsigned long, std::string> &patterns, unsigned int workers,
                             std::function<std::shared_ptr<BaseSimilarityScorer>(unsigned long, std::string)> scorer_constructor,
                             bool computeMatrix = true);
    ~SimilarityTable();

    void to_similarity_graph();
    std::string to_abc();
    std::string to_abc(const std::string &abc_graph_output);
    void prune(double threshold);
    /**
     * Only keeps the top-k highest edges for a given node
     * @param edges Number of edges to keep
     */
    void top_k_edges(unsigned int edges);
    std::optional<std::string> get_pattern(unsigned long id) const;

    const std::vector<std::vector<double>> &get_scores() const {
        return scores;
    }

    const std::vector<std::shared_ptr<BaseSimilarityScorer>> &get_scorers() const {
        return scorers;
    }

private:
    std::vector<std::vector<double>> scores;
    std::vector<std::shared_ptr<BaseSimilarityScorer>> scorers;
    std::string abc_file;
    bool has_temp_abc_file;
};


#endif //_SIMILARITY_TABLE_H
