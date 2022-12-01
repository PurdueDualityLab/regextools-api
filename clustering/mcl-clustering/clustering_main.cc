//
// Created by charlie on 2/9/22.
//

#include "similarity_table/similarity_table.h"
#include "mcl_wrapper.h"
#include "cluster_set.h"
#include "librereuse/db/pattern_reader.h"
#include "program_options.h"
#include "similarity_table/egret_similarity_scorer.h"

#include <iostream>
#include <unordered_map>
#include <optional>
#include <getopt.h>
#include "re2/re2.h"

static int help;

static const struct option program_args[] = {
        { "spec", required_argument, nullptr, 'f' },
        { "inflation", required_argument, nullptr, 'i' },
        { "pruning", required_argument, nullptr, 'p' },
        { "graph-out", required_argument, nullptr, 'g' },
        { "cluster-out", required_argument, nullptr, 'o' },
        { "patterns-file", required_argument, nullptr, 'P' },
        { "parallel", required_argument, nullptr, 'j' },
        { "corpus-type", required_argument, nullptr, 't'},
        { "scorer", required_argument, nullptr, 's' },
        { "top-k-edges", required_argument, nullptr, 'k' },
        { "strict-string-checking", no_argument, nullptr, '1' },
        { "existing-graph", required_argument, nullptr, '2' },
        { "help", no_argument, nullptr, 'h' },
        { nullptr, 0, nullptr, 0 }
};

static void display_help() {
    std::cout << "clustering - cluster a corpus of regexes into semantic clusters\n";
    std::cout << "usage: clustering [options] [corpus_file.txt]\n";
    std::cout << "\n";
    std::cout << "options:\n";
    std::cout << "-f, --spec:         sets a spec file definition. The spec file is a json file that contains all of\n"
                "                     these options (note: dashes in these options should be replaced with underscores (_)\n"
                "                     in the spec file). If this file is used, all other specified params will be ignored\n"
                "                     if overwritten\n";
    std::cout << "-i, --inflation:    set the mcl inflation parameter (default is 1.8)\n";
    std::cout << "-p, --pruning:      set the mcl pruning parameter (default is off)\n";
    std::cout << "-g, --graph-out:    path to a file to write out the resulting similarity graph\n";
    std::cout << "-o, --cluster-out:  path to a file to write the resulting cluster with ids (default is clusters.txt)\n";
    std::cout << "-P, --patterns-out: file to write the resulting clusters to, ids mapped to patterns\n";
    std::cout << "-j, --parallel:     how many workers to work with (NOTE: default is all available cores in the computer)\n";
    std::cout << "-s, --scorer:       what kind of scorer to use for language approximation. 'rex' (default), 'egret'\n";
    std::cout << "-k, --top-k-edges:  only keep the top k edges\n";
    std::cout << "-t, --corpus-type:  how the patterns are represented in the corpus. options are 'objects' or 'pairs'\n"
              << "                    'objects' are json object per line, each object has a pattern kv-pair.\n"
              << "                    'pairs' are id, whitespace then pattern. (default)\n"
              << "                    'clusters' is a json array of arrays where each subarray is a cluster\n";
    std::cout << "--existing-graph:   use a similarity graph that was already used for the given corpus. If this option\n"
                 "                    is set, then computing the similarity matrix will be skipped. The program will just\n"
                 "                    cluster the existing graph and map the clusters back to patterns. NOTE: you must still\n"
                 "                    provide the corresponding corpus file so that the IDs can be mapped back to the graph.\n";
    std::cout << '\n';
    std::cout << "--strict-string-checking: strictly check for corruptions in rex strings when being loaded and unloaded\n";
    std::cout << "-h, --help:         display this help screen" << std::endl;
}

static ProgramOptions read_program_opts(int argc, char **argv) {
    const char *getopt_str = "p:i:g:o:P:j:t:s:k:f:h";
    int c;
    int opt_index;
    ProgramOptions option_values;
    while ((c = getopt_long(argc, argv, getopt_str, program_args, &opt_index)) != -1) {
        switch (c) {
            case 'h':
                // help = 1;
                display_help();
                exit(0);

            case 'f':
                option_values.spec_file = std::string(optarg);
                break;

            case 'i': {
                float inflation_value = std::stof(std::string(optarg));
                option_values.inflation = static_cast<double>(inflation_value);
                break;
            }

            case 'p': {
                float pruning_value = std::stof(std::string(optarg));
                option_values.pruning = static_cast<double>(pruning_value);
                break;
            }

            case 'g':
                option_values.graph_out = std::string(optarg);
                break;

            case 'o':
                option_values.cluster_out = std::string(optarg);
                break;

            case 'P':
                option_values.patterns_file_out = std::string(optarg);
                break;

            case 'j':
                option_values.workers = std::stoi(std::string(optarg));
                break;

            case 't': {
                std::string option(optarg);
                if (option == "objects") {
                    option_values.corpus_type = CorpusType::OBJECTS;
                } else if (option == "pairs") {
                    option_values.corpus_type = CorpusType::PAIRS;
                } else if (option == "clusters") {
                    option_values.corpus_type = CorpusType::CLUSTERS;
                } else {
                    throw std::runtime_error("Invalid corpus type");
                }
                break;
            }

            case 's': {
                std::string option(optarg);
                if (option == "egret") {
                    option_values.scorer_type = ScorerType::EGRET;
                } else if (option == "rex") {
                    option_values.scorer_type = ScorerType::REX;
                } else {
                    throw std::runtime_error("Invalid scorer type");
                }
                break;
            }

            case 'k': {
                long k_edges = std::stol(std::string(optarg));
                if (k_edges < 0)
                    throw std::runtime_error("Edges cannot be negative");

                option_values.top_k_edges = static_cast<unsigned int>(k_edges);
                break;
            }

            case '1':
                option_values.strict_rex_string_checking = true;
                break;

            case '2':
                option_values.existing_graph_path = std::string(optarg);
                break;

            default:
                throw std::runtime_error("Unexpected argument");
        }
    }

    // get the last positional argument
    try {
        option_values.corpus_file = std::string(argv[optind]);
    } catch (std::logic_error &logic_err) {
        throw std::runtime_error("No corpus file provided");
    }

    return option_values;
}

int main(int argc, char **argv) {

    // read arguments
    ProgramOptions program_arguments {};
    try {
        program_arguments = read_program_opts(argc, argv);
    } catch (std::runtime_error &exe) {
        std::cerr << "Error while parsing program options: " << exe.what() << std::endl;
        std::cerr << "Please consult 'clustering --help' for usage help" << std::endl;
        return 1;
    }

    ProgramOptions::set_instance(std::move(program_arguments));

    // Patch from spec file instead
    if (ProgramOptions::instance().spec_file)
        ProgramOptions::instance().patch_from_spec_file(*ProgramOptions::instance().spec_file);

    // read in all the patterns
    std::unordered_map<unsigned long, std::string> patterns;
    if (ProgramOptions::instance().corpus_type == CorpusType::PAIRS)
        // Load pairs
        patterns = rereuse::db::read_patterns_id_pairs_path(ProgramOptions::instance().corpus_file);
    else if (ProgramOptions::instance().corpus_type == CorpusType::CLUSTERS) {
        auto clusters = rereuse::db::read_semantic_clusters(ProgramOptions::instance().corpus_file);
        unsigned long id = 0;
        for (auto &cluster : clusters) {
            for (auto &pattern : cluster->get_patterns()) {
                patterns[id++] = pattern;
            }
        }
    } else {
        // Load objects
        auto pattern_list = rereuse::db::read_patterns_from_path(ProgramOptions::instance().corpus_file);
        unsigned long id = 0;
        for (auto & pattern : pattern_list) {
            patterns[id++] = std::move(pattern);
        }
    }

    MclWrapper mcl_wrapper(ProgramOptions::instance().mcl_path);

    // build the similarity table, but don't actually compute it
    std::function<std::shared_ptr<BaseSimilarityScorer>(unsigned long, std::string)> scorer_constructor;
    if (ProgramOptions::instance().scorer_type == REX) {
        RexWrapper rex_wrapper(ProgramOptions::instance().rex_path, ProgramOptions::instance().wine_path);
        scorer_constructor = [rex_wrapper](unsigned long id, const std::string &pattern) {
            try {
                return std::shared_ptr<BaseSimilarityScorer>(new RexSimilarityScorer(pattern, id, rex_wrapper));
            } catch (std::runtime_error &exe) {
                return std::shared_ptr<BaseSimilarityScorer>();
            }
        };
    } else {
        scorer_constructor = [](unsigned long id, const std::string &pattern) {
            try {
                return std::shared_ptr<BaseSimilarityScorer>(new EgretSimilarityScorer(pattern, id));
            } catch (std::runtime_error &exe) {
                // Return null if there is an error
                return std::shared_ptr<BaseSimilarityScorer>();
            }
        };
    }

    SimilarityTable table(patterns, ProgramOptions::instance().workers, scorer_constructor, !ProgramOptions::instance().existing_graph_path.has_value());

    // If an existing graph is passed, then skip generating the similarity table
    if (!ProgramOptions::instance().existing_graph_path) {
        // Create similarity graph
        table.to_similarity_graph();

        // Next line is optional. This prunes before going to mcl
        if (ProgramOptions::instance().pruning > 0) {
            std::cout << "Pruning values below " << ProgramOptions::instance().pruning << std::endl;
            table.prune(ProgramOptions::instance().pruning);
        }
    }
    // ...to here is the scoring portion

    std::string abc_graph;
    if (ProgramOptions::instance().existing_graph_path)
        abc_graph = *ProgramOptions::instance().existing_graph_path;
    else if (ProgramOptions::instance().graph_out)
        abc_graph = table.to_abc(*ProgramOptions::instance().graph_out);
    else
        abc_graph = table.to_abc();

    std::vector<std::vector<unsigned long>> raw_clusters;
    if (ProgramOptions::instance().cluster_out) {
        raw_clusters = mcl_wrapper.cluster(abc_graph, ProgramOptions::instance().inflation,
                                           ProgramOptions::instance().pruning, ProgramOptions::instance().top_k_edges,
                                           *ProgramOptions::instance().cluster_out);
    } else {
        raw_clusters = mcl_wrapper.cluster(abc_graph, ProgramOptions::instance().inflation,
                                           ProgramOptions::instance().pruning, ProgramOptions::instance().top_k_edges);
    }

    ClusterSet cluster_set(raw_clusters);
    std::cout << cluster_set << std::endl;
    if (ProgramOptions::instance().patterns_file_out) {
        std::ofstream patterns_file(*ProgramOptions::instance().patterns_file_out);
        cluster_set.write_patterns(table, patterns_file);
        patterns_file.close();
    } else {
        std::cout << "\n\n\n" << std::endl;
        cluster_set.write_patterns(table, std::cout);
        std::cout << std::endl;
    }

    return 0;
}
