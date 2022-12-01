//
// Created by charlie on 2/14/22.
//

#ifndef _EGRET_SIMILARITY_SCORER_H
#define _EGRET_SIMILARITY_SCORER_H

#include "base_similarity_scorer.h"

#include <unordered_set>
#include "re2/re2.h"

class EgretSimilarityScorer: public BaseSimilarityScorer {
public:
    explicit EgretSimilarityScorer(std::string pattern, unsigned long id);
    ~EgretSimilarityScorer() override = default;

    double score(std::shared_ptr<BaseSimilarityScorer> other_scorer) override;
    bool test_string(const std::string &subject, bool should_match) const;

private:
    std::unordered_set<std::string> positive;
    std::unordered_set<std::string> negative;
    std::unique_ptr<re2::RE2> regex;
};


#endif //_EGRET_SIMILARITY_SCORER_H
