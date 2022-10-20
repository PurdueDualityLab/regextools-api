//
// Created by charlie on 10/10/22.
//

#ifndef REGEXTOOLS_ARG_PARSER_H
#define REGEXTOOLS_ARG_PARSER_H

#include <string>
#include <unordered_map>
#include <memory>
#include "librereuse/query/base_cluster_query.h"
#include "librereuse/query/base_regex_query.h"

struct ProgramArgs {
    ProgramArgs(int argc, char **argv);

    bool showHelp;
    std::string spec_file;
};

struct BenchmarkingSpec {
    explicit BenchmarkingSpec(const std::string &spec_file_path);

    std::string db_file_path;
    // Nice going...
    std::unordered_map<
        std::string,
        std::pair<
                std::unique_ptr<rereuse::query::BaseClusterQuery>,
                std::unique_ptr<rereuse::query::BaseRegexQuery>
                >
        > queries;
};

#endif //REGEXTOOLS_ARG_PARSER_H
