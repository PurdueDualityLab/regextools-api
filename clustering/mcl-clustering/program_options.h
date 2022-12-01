//
// Created by charlie on 2/15/22.
//

#ifndef _PROGRAM_OPTIONS_H
#define _PROGRAM_OPTIONS_H

#include <string>
#include <optional>
#include <thread>
#include <fstream>
#include <any>
#include "nlohmann/json.hpp"

enum CorpusType {
    OBJECTS, // json object per line, each object has a pattern key-value pair
    PAIRS, // Each line has an id, whitespace, and then patterns
    CLUSTERS, // json array of arrays. Each subarray is a cluster
};

NLOHMANN_JSON_SERIALIZE_ENUM(CorpusType, {
    { OBJECTS, "objects" },
    { PAIRS, "pairs" },
    { CLUSTERS, "clusters" }
})

enum ScorerType {
    REX, // use rex for string generation
    EGRET, // use egret for string generation
};

NLOHMANN_JSON_SERIALIZE_ENUM(ScorerType, {
    { REX, "rex" },
    { EGRET, "egret" }
})

class ProgramOptions {
public:
    static ProgramOptions &instance() {
        return ProgramOptions::global_options_instance;
    }

    static void set_instance(ProgramOptions &&options) {
        ProgramOptions::global_options_instance = std::move(options);
    }

    ProgramOptions()
            : inflation(1.8)
            , pruning(0)
            // default to using all available cores. If running on a big computer, specify something else
            , workers(std::thread::hardware_concurrency())
            , corpus_type(CorpusType::PAIRS)
            , strict_rex_string_checking(false)
            , scorer_type(ScorerType::REX)
            , top_k_edges(0)
            , wine_path("/usr/bin/wine")
            , rex_path("/home/charlie/Downloads/Rex/Rex.exe")
            , mcl_path("/usr/local/bin/mcl")
    {  }

    friend std::ostream &operator<<(std::ostream &os, const ProgramOptions &opts);

    void patch_from_spec_file(const std::string &spec_path);

    double inflation;
    double pruning;
    std::optional<std::string> graph_out;
    std::optional<std::string> cluster_out;
    std::optional<std::string> patterns_file_out;
    std::optional<std::string> spec_file;
    std::string corpus_file;
    unsigned int workers;
    CorpusType corpus_type;
    bool strict_rex_string_checking;
    ScorerType scorer_type;
    unsigned int top_k_edges;
    std::string wine_path;
    std::string rex_path;
    std::string mcl_path;
    std::optional<std::string> existing_graph_path;

    static ProgramOptions global_options_instance;
};


#endif //_PROGRAM_OPTIONS_H
