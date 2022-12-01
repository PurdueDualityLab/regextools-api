//
// Created by charlie on 2/10/22.
//

#include "cluster_set.h"
#include "nlohmann/json.hpp"

#include <fstream>

ClusterSet::ClusterSet(const std::string &cluster_file_path) {
    std::ifstream cluster_file(cluster_file_path);
    re2::RE2 line_parser("(\\d+)\\s*");
    std::string line;
    while (std::getline(cluster_file, line)) {
        re2::StringPiece line_str_piece(line);
        unsigned long id;
        std::unordered_set<unsigned long> cluster;
        while (re2::RE2::FindAndConsume(&line_str_piece, line_parser, &id)) {
            cluster.insert(id);
        }

        this->clusters.push_back(std::move(cluster));
    }

    cluster_file.close();
}

ClusterSet::ClusterSet(const std::vector<std::vector<unsigned long>> &clusters) {
    for (auto &cluster : clusters) {
        std::unordered_set<unsigned long> cluster_set;
        std::copy(cluster.cbegin(),  cluster.cend(), std::inserter(cluster_set, cluster_set.begin()));
        this->clusters.push_back(std::move(cluster_set));
    }
}

std::ostream &operator<<(std::ostream &os, const ClusterSet &set) {
    os << "{\n";
    for (const auto &cluster : set.get_clusters()) {
        os << '[';
        auto it = cluster.begin();
        for (int i = 0; i < cluster.size() - 1; i++) {
            os << *it << ", ";
            std::advance(it, 1);
        }
        os << *it << "]\n";
    }
    os << '}';
    os.flush();

    return os;
}

void ClusterSet::write_patterns(const SimilarityTable &table, std::ostream &out) const {
    nlohmann::json clusters_obj;
    // Get all the patterns
    for (const auto &cluster : this->clusters) {
        std::unordered_set<std::optional<std::string>> pattern_set_opts;
        std::unordered_set<std::string> patterns;
        // Translate id to pattern
        std::transform(cluster.begin(), cluster.end(), std::inserter(pattern_set_opts, pattern_set_opts.begin()),
                       [&table] (unsigned long id) { return table.get_pattern(id); });

        // Remove empty optionals
        for (auto it = pattern_set_opts.begin(); it != pattern_set_opts.end();) {
            if (!it->has_value()) {
                it = pattern_set_opts.erase(it);
            } else {
                ++it;
            }
        }

        // Unwrap the optionals
        std::transform(pattern_set_opts.begin(), pattern_set_opts.end(), std::inserter(patterns, patterns.begin()),
                       [] (const std::optional<std::string> &pattern_opt) { return pattern_opt.value(); });

        // Add this cluster to the array
        clusters_obj.push_back(patterns);
    }

    // Write out the object to the stream
    out << clusters_obj;
}

