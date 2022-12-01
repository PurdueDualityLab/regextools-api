//
// Created by charlie on 2/10/22.
//

#include "rex_similarity_scorer.h"

#include "nlohmann/json.hpp"
#include "re2/re2.h"
#include "../program_options.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>

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
: BaseSimilarityScorer(pattern, id) {

    // Generate strings for this pattern
    std::vector<std::string> strings;
    try {
        strings = rex_wrapper.generate_strings(pattern, 400);
    } catch (std::runtime_error &exe) {
        throw exe;
    }

    if (ProgramOptions::instance().strict_rex_string_checking) {
        std::cout << "Strictly checking rex strings" << std::endl;
        RexStringsHasher rex_strings_hasher;
        this->strings_hash = rex_strings_hasher(strings);
    }

    nlohmann::json strings_obj;
    for (auto &str : strings)
        strings_obj.push_back(str);

    // open a new file
    this->strings_file_path = "/tmp/rex_strings" + std::to_string(random()) + ".json";
    std::ofstream strings_file(this->strings_file_path);
    strings_file << strings_obj;

    // Build a pattern for this regex
    // this->regex = std::make_unique<re2::RE2>(pattern);
    re2::RE2 test_regex(pattern); // This should get dropped
    if (!test_regex.ok()) {
        throw std::runtime_error("Regex is not supported by re2");
    }
}

double RexSimilarityScorer::score(std::shared_ptr<BaseSimilarityScorer> other_scorer) {
    // TODO the smarter way to do this would be to have a regex pointer in the base scorer, but that's
    // not necessarily general
    auto rex_scorer = std::dynamic_pointer_cast<RexSimilarityScorer>(other_scorer);
    double hits = 0;
    auto strings = this->load_strings();
    re2::RE2 other_regex(other_scorer->get_pattern());
    for (const auto &str : strings) {
        if (re2::RE2::PartialMatch(str, other_regex)) {
            hits++;
        }
    }

    return hits / (double) strings.size();
}

std::vector<std::string> RexSimilarityScorer::load_strings() {
    int strings_file_fd = open(this->strings_file_path.c_str(), O_RDONLY);
    if (strings_file_fd < 0) {
        throw std::runtime_error("Could not read strings file");
    }

    struct stat strings_file_stats;
    if (fstat(strings_file_fd, &strings_file_stats) == -1) {
        throw std::runtime_error("Could not stat strings file");
    }

    size_t file_length = strings_file_stats.st_size;

    // Load the strings into data
    char *strings_buffer_raw = static_cast<char *>(mmap(nullptr, file_length, PROT_READ, MAP_PRIVATE, strings_file_fd, 0));
    if (strings_buffer_raw == MAP_FAILED) {
        throw std::runtime_error("Failed to map file with errno: " + std::to_string(errno));
    }
    std::string strings_buffer(strings_buffer_raw, file_length);

    // Parse the strings via json
    std::vector<std::string> strings;
    try {
        strings = nlohmann::json::parse(strings_buffer).get<std::vector<std::string>>();
    } catch (nlohmann::json::parse_error &err) {
        throw std::runtime_error("Error while parsing strings buffer");
    } catch (nlohmann::json::type_error &type_err) {
        throw std::runtime_error("Strings could not be loaded because of mismatched types");
    }

    munmap(strings_buffer_raw, file_length);
    close(strings_file_fd);

    // Check if the strings are the same
    RexStringsHasher rex_strings_hasher;
    if (ProgramOptions::instance().strict_rex_string_checking && rex_strings_hasher(strings) != this->strings_hash) {
        std::cerr << "WARNING: strings loaded did not match the original hash value" << std::endl;
    }

    return strings;
}

RexSimilarityScorer::~RexSimilarityScorer() {
    int ret = unlink(this->strings_file_path.c_str());
    if (ret < 0) {
        std::cerr << "Error while deleting strings file: ";
        perror("unlink");
    }
}

bool RexSimilarityScorer::test_string(const std::string &subject) const {
    re2::RE2 this_regex(this->pattern);
    return re2::RE2::PartialMatch(subject, this_regex);
}
