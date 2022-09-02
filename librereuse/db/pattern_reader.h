//
// Created by charlie on 9/19/21.
//

#ifndef _PATTERNREADER_H
#define _PATTERNREADER_H

#include "cluster.h"

#include <istream>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

namespace rereuse::db {
    std::vector<std::string> read_patterns(std::istream &input_stream);
    std::vector<std::string> read_patterns_from_path(const std::string &path);
    std::unordered_map<unsigned long, std::string> read_patterns_id_pairs(std::istream &input_stream);
    std::unordered_map<unsigned long, std::string> read_patterns_id_pairs_path(const std::string &path);
    std::vector<std::unique_ptr<rereuse::db::Cluster>> read_semantic_clusters(const std::string &file_path);
}

#endif //_PATTERNREADER_H
