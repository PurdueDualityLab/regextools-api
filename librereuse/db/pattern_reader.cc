//
// Created by charlie on 9/19/21.
//

#include "pattern_reader.h"

#include <fstream>

#include <nlohmann/json.hpp>

using namespace nlohmann;

std::vector<std::string> rereuse::db::read_patterns(std::istream &input_stream) {
    std::vector<std::string> patterns;
    std::string line;
    while (std::getline(input_stream, line)) {
        try {
            auto line_obj = json::parse(line);
            auto pattern = line_obj["pattern"];
            // TODO Filter out super long patterns maybe?
            patterns.push_back(std::move(pattern));
        } catch (json::parse_error &err) {
            // Just keep going
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
        return {};
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
