//
// Created by charlie on 2/10/22.
//

#ifndef _REX_SIMILARITY_SCORER_H
#define _REX_SIMILARITY_SCORER_H

#include "../rex_wrapper.h"

#include <memory>
#include <filesystem>
#include "re2/re2.h"

struct RexStringsHasher {
    std::size_t operator()(const std::vector<std::string> &strings) const;
};

class RexSimilarityScorer {
public:
    RexSimilarityScorer(const std::string &pattern, unsigned long id, const RexWrapper &rex_wrapper);
    ~RexSimilarityScorer();
    double score(const RexSimilarityScorer &other_scorer);

    bool test_string(const std::string &subject) const;

    unsigned long get_id() const { return this->id; }

    const std::string &get_pattern() const { return this->pattern; }

private:
    std::vector<std::string> load_strings();

    std::string pattern;
    std::filesystem::path string_file_path;
    std::unique_ptr<re2::RE2> regex;
    std::size_t strings_hash;
    unsigned long id;
};


#endif //_REX_SIMILARITY_SCORER_H
