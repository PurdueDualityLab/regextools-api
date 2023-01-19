//
// Created by charlie on 9/19/21.
//

#include "pattern_reader.h"

#include <fstream>

#include <nlohmann/json.hpp>
#include <random>
#include "spdlog/spdlog.h"

using namespace nlohmann;

std::vector<std::string> rereuse::db::read_patterns(std::istream &input_stream) {
    std::vector<std::string> patterns;
    std::string line;
    unsigned long line_no = 0;
    while (std::getline(input_stream, line)) {
        line_no++;
        try {
            auto line_obj = json::parse(line);
            auto pattern = line_obj["pattern"];
            // TODO Filter out super long patterns maybe?
            patterns.push_back(std::move(pattern));
        } catch (json::parse_error &err) {
            // Just keep going
            spdlog::debug("pattern_reader:read_patterns: Failed to parse line {}", line_no);
            continue;
        }
    }

    return patterns;
}

std::vector<std::string> rereuse::db::read_patterns_from_path(const std::string &path) {
    std::ifstream db_file(path);
    if (db_file.is_open()) {
        std::vector<std::string> patterns = rereuse::db::read_patterns(db_file);
        db_file.close();
        return patterns;
    } else {
        return {};
    }
}

static inline void trim_string_end(std::string &str) {
    str.erase(std::find_if(str.rbegin(), str.rend(),
                           [](unsigned char c) { return !std::isspace(c); }).base(), str.end());
}

std::unordered_map<unsigned long, std::string> rereuse::db::read_patterns_id_pairs(std::istream &input_stream) {
    re2::RE2 parser("^(\\d+)\\s+(.*)$");
    std::string line;
    std::unordered_map<unsigned long, std::string> patterns;
    while (std::getline(input_stream, line)) {
        unsigned long id;
        std::string pattern;
        if (re2::RE2::FullMatch(line, parser, &id, &pattern)) {
            // Trim any whitespace on the end of pattern
            trim_string_end(pattern);
            patterns[id] = pattern;
        }
    }

    return patterns;
}

std::unordered_map<unsigned long, std::string> rereuse::db::read_patterns_id_pairs_path(const std::string &path) {
    std::ifstream in_file(path);
    if (!in_file)
        throw std::runtime_error("Could not open patterns file");

    auto patterns = read_patterns_id_pairs(in_file);
    in_file.close();
    return patterns;
}

std::vector<std::unique_ptr<rereuse::db::Cluster>> rereuse::db::read_semantic_clusters(const std::string &file_path) {
    std::ifstream seed_file(file_path);
    nlohmann::json cluster_array;
    if (seed_file.is_open()) {
        seed_file >> cluster_array;
        seed_file.close();
    } else {
        spdlog::error("pattern_readers:read_semantic_clusters: could not open cluster file");
        throw std::runtime_error("Could not open semantic cluster file");
    }

    std::vector<std::unique_ptr<rereuse::db::Cluster>> clusters;

    // Iterate over clusters
    for (const auto &cluster_patterns : cluster_array) {
        std::unordered_set<std::string> patterns;
        for (const auto &cluster_pattern : cluster_patterns) {
            patterns.insert(cluster_pattern.get<std::string>());
        }

        auto cluster = std::make_unique<rereuse::db::Cluster>(patterns);
        clusters.push_back(std::move(cluster));
    }

    return clusters;
}

std::vector<std::string> rereuse::db::unpack_patterns(const std::vector<std::unique_ptr<rereuse::db::Cluster>> &clusters) {
    std::vector<std::string> all_patterns;
    for (const auto &cluster : clusters) {
        const auto &patterns = cluster->get_entities();
        std::copy(patterns.cbegin(), patterns.cend(), std::back_inserter(all_patterns));
    }

    return all_patterns;
}


std::vector<std::unique_ptr<rereuse::db::Cluster>> rereuse::db::randomize_clusters(const std::vector<std::string> &all_patterns_orig, unsigned int expected_cluster_size) {
    // Copy all the patterns
    std::vector<std::string> all_patterns(all_patterns_orig.cbegin(), all_patterns_orig.cend());
    // Shuffle their order
    std::shuffle(all_patterns.begin(), all_patterns.end(), std::mt19937(std::random_device()()));
    spdlog::debug("randomized_cluster: creating clusters of size {}", expected_cluster_size);

    std::vector<std::unique_ptr<rereuse::db::Cluster>> new_clusters;
    std::unordered_set<std::string> cluster_strings;
    for (auto &pattern : all_patterns) {
        // Add this item to the current cluster strings
        cluster_strings.insert(std::move(pattern));

        if (cluster_strings.size() == expected_cluster_size) {
            // This cluster is full
            auto new_cluster = std::make_unique<rereuse::db::Cluster>(cluster_strings);
            // Clear the set
            cluster_strings.clear();

            new_clusters.push_back(std::move(new_cluster));
        }
    }

    return new_clusters;
}
