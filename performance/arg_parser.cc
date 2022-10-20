//
// Created by charlie on 10/10/22.
//

#include "arg_parser.h"
#include "librereuse/query/cluster_match_query.h"
#include "librereuse/query/match_query.h"

#include <getopt.h>
#include <stdexcept>
#include <fstream>
#include <nlohmann/json.hpp>

const static option program_options[] = {
        { "--help", no_argument, nullptr, 'h' },
        {nullptr, 0, nullptr, 0},
};

ProgramArgs::ProgramArgs(int argc, char **argv)
: showHelp(false) {
    int idx;
    int c;
    while ((c = getopt_long(argc, argv, "h", program_options, &idx)) > 0) {
        switch (c) {
            case 'h':
                this->showHelp = true;
                break;

            default:
                throw std::runtime_error("Unexpected argument");
        }
    }

    for (idx = optind; idx < argc; idx++) {
        this->spec_file = std::string(argv[idx]);
    }
}

static std::unordered_map<
        std::string,
        std::pair<
                std::unique_ptr<rereuse::query::BaseClusterQuery>,
                std::unique_ptr<rereuse::query::BaseRegexQuery>
        >
> break_down_queries(const nlohmann::json& queries_obj) {
    std::unordered_map<
            std::string,
            std::pair<
                    std::unique_ptr<rereuse::query::BaseClusterQuery>,
                    std::unique_ptr<rereuse::query::BaseRegexQuery>
            >
    > queries;
    for (const auto &query_obj: queries_obj) {
        std::unordered_set<std::string> positive, negative;
        std::copy(query_obj.at("positive").cbegin(), query_obj.at("positive").cend(), std::inserter(positive, positive.begin()));
        std::copy(query_obj.at("negative").cbegin(), query_obj.at("negative").cend(), std::inserter(negative, negative.begin()));
        auto query_name = query_obj.at("name").get<std::string>();
        queries[query_name] = std::make_pair(
                std::make_unique<rereuse::query::ClusterMatchQuery>(positive, negative),
                std::make_unique<rereuse::query::MatchQuery>(positive, negative)
                );
    }

    return queries;
}

BenchmarkingSpec::BenchmarkingSpec(const std::string &spec_file_path) {
    std::ifstream input_file(spec_file_path);
    nlohmann::json spec_obj;
    input_file >> spec_obj;

    this->db_file_path = spec_obj.at("cluster_file");

    auto queries_list = spec_obj.at("queries");
    this->queries = break_down_queries(queries_list);
}
