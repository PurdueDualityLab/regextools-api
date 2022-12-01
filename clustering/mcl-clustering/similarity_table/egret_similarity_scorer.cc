//
// Created by charlie on 2/14/22.
//

#include "egret_similarity_scorer.h"

#include <utility>

#include "src/egret.h"

EgretSimilarityScorer::EgretSimilarityScorer(std::string pattern, unsigned long id)
: BaseSimilarityScorer(std::move(pattern), id) {

    // Use egret to generate strings
    auto strings = run_engine(this->pattern, "evil");

    // Build out regex
    this->regex = std::make_unique<re2::RE2>(this->pattern);
    if (!this->regex->ok())
        throw std::runtime_error("Failed to build regex for scorer");

    // See which string belongs where
    for (auto &str : strings) {
        if (re2::RE2::FullMatch(str, *this->regex)) {
            this->positive.insert(std::move(str));
        } else {
            this->negative.insert(std::move(str));
        }
    }
}

double EgretSimilarityScorer::score(std::shared_ptr<BaseSimilarityScorer> other_scorer) {

    auto scorer = std::static_pointer_cast<EgretSimilarityScorer>(other_scorer);
    size_t lang_size = this->positive.size() + this->negative.size();
    size_t hits = 0;

    for (const auto &pos : this->positive) {
        if (scorer->test_string(pos, true))
            hits++;
    }

    for (const auto &neg : this->negative) {
        if (scorer->test_string(neg, false))
            hits++;
    }

    return (double) hits / (double) lang_size;
}

bool EgretSimilarityScorer::test_string(const std::string &subject, bool should_match) const {
    return should_match ?
    re2::RE2::PartialMatch(subject, *this->regex)
    : !re2::RE2::PartialMatch(subject, *this->regex);
}
