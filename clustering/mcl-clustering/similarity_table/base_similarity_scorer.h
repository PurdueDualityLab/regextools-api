//
// Created by charlie on 2/10/22.
//

#ifndef _BASE_SIMILARITY_SCORER_H
#define _BASE_SIMILARITY_SCORER_H

#include <string>
#include <memory>
#include <stdexcept>

class BaseSimilarityScorer {
protected:
    explicit BaseSimilarityScorer(std::string pattern, unsigned long id = 0)
    : pattern(std::move(pattern))
    , id(id) {
        if (this->pattern.empty())
            throw std::runtime_error("Pattern must not be empty");
    }

    virtual ~BaseSimilarityScorer() = default;

    std::string pattern;
    unsigned long id;

public:
    virtual double score(std::shared_ptr<BaseSimilarityScorer> other_scorer) = 0;

    const std::string &get_pattern() const {
        return this->pattern;
    }

    unsigned long get_id() const {
        return id;
    }
};


#endif //_BASE_SIMILARITY_SCORER_H
