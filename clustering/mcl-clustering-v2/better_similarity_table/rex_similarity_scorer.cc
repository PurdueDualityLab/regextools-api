//
// Created by charlie on 2/10/22.
//

#include "rex_similarity_scorer.h"

#include "nlohmann/json.hpp"
#include "re2/re2.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <spdlog/spdlog.h>

// Shamelessly stolen from here: https://stackoverflow.com/a/2595226/9421263
static inline void hash_combine(std::size_t& seed, const std::size_t &added_hash) {
    seed ^= added_hash + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

std::size_t RexStringsHasher::operator()(const std::vector<std::string> &strings) const {
    std::vector<std::size_t> string_hashes(strings.size());
    std::hash<std::string> string_hasher;
    for (const auto &str : strings) {
        string_hashes.push_back(string_hasher(str));
    }

    // Combine all the hashes
    std::size_t strings_hash = string_hashes[0];
    string_hashes.erase(string_hashes.begin());
    for (const auto &h_val : string_hashes) {
        hash_combine(strings_hash, h_val);
    }

    return strings_hash;
}

RexSimilarityScorer::RexSimilarityScorer(const std::string &pattern, unsigned long id, const RexWrapper &rex_wrapper)
: pattern(pattern)
, id(id) {
    // Generate strings for this pattern
    std::vector<std::string> strings;
    try {
        strings = rex_wrapper.generate_strings(pattern, 400);
    } catch (std::runtime_error &exe) {
        throw exe;
    }

    nlohmann::json strings_obj;
    for (auto &str : strings)
        strings_obj.push_back(str);

    // open a new file
    auto rex_strings_path = std::filesystem::temp_directory_path() / "rex_strings";
    std::error_code err;
    bool result = std::filesystem::create_directories(rex_strings_path, err);
    if (!result && err) {
        throw std::runtime_error("Could not create strings directory: " + err.message());
    }

    this->string_file_path = rex_strings_path / ("rex_strings_" + std::to_string(random()) + ".json");
    std::ofstream strings_file(this->string_file_path);
    if (!strings_file.is_open()) {
        throw std::runtime_error("Could not create strings file");
    }
    strings_file << strings_obj;
    strings_file.flush();

    // Build a pattern for this regex
    re2::RE2 test_regex(pattern); // This should get dropped
    if (!test_regex.ok()) {
        throw std::runtime_error("Regex is not supported by re2");
    }
}

double RexSimilarityScorer::score(const RexSimilarityScorer &other_scorer) {
    double hits = 0;
    auto strings = this->load_strings();
    re2::RE2 other_regex(other_scorer.get_pattern());
    for (const auto &str : strings) {
        if (re2::RE2::PartialMatch(str, other_regex)) {
            hits++;
        }
    }

    return hits / (double) strings.size();
}

std::vector<std::string> RexSimilarityScorer::load_strings() {
    std::ifstream input(this->string_file_path);
    nlohmann::json strings_obj;
    input >> strings_obj;

    return strings_obj.get<std::vector<std::string>>();
}

RexSimilarityScorer::~RexSimilarityScorer() {
    spdlog::warn("RexSimilarityScorer: cleaning up strings file");
    std::filesystem::remove(this->string_file_path);
}

bool RexSimilarityScorer::test_string(const std::string &subject) const {
    re2::RE2 this_regex(this->pattern);
    return re2::RE2::PartialMatch(subject, this_regex);
}
