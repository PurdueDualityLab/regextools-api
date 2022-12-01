//
// Created by charlie on 2/15/22.
//

#include "program_options.h"
#include <ostream>

ProgramOptions ProgramOptions::global_options_instance;

std::ostream &operator<<(std::ostream &os, const ProgramOptions &opts) {
    os << "OPTIONS\n";
    os << "Inflation: " << opts.inflation << '\n';
    os << "Pruning: " << opts.pruning << '\n';
    os << "Graph Out: ";
    if (opts.graph_out)
        os << *opts.graph_out;
    else
        os << "<none>";
    os << '\n';
    os << "Cluster Out: ";
    if (opts.cluster_out)
        os << *opts.cluster_out;
    else
        os << "<none>";
    os << '\n';
    os << "Patterns file out: ";
    if (opts.patterns_file_out)
        os << *opts.patterns_file_out;
    else
        os << "<none>";
    os << '\n';
    os << "Corpus file: " << opts.corpus_file << '\n';
    os << "Workers: " << opts.workers << '\n';
    os << "Scorer type: ";
    if (opts.scorer_type == ScorerType::REX) {
        os << "Rex";
    } else {
        os << "Egret";
    }
    os << '\n';
    os << "Corpus type: ";
    if (opts.corpus_type == CorpusType::CLUSTERS)
        os << "CLUSTERS";
    else if (opts.corpus_type == CorpusType::PAIRS)
        os << "PAIRS";
    else
        os << "OBJECTS";
    os << '\n';
    os << "Strict rex string checking: " << opts.strict_rex_string_checking << '\n';

    return os;
}

void ProgramOptions::patch_from_spec_file(const std::string &spec_path) {
    // Read spec object from
    std::ifstream spec_input_file(spec_path);
    nlohmann::json spec;
    spec_input_file >> spec;
    spec_input_file.close();

    this->inflation = spec.value("inflation", this->inflation);
    this->pruning = spec.value("pruning", this->pruning);
    if (spec.contains("graph_out"))
        this->graph_out = spec.value("graph_out", *this->graph_out);
    if (spec.contains("cluster_out"))
        this->cluster_out = spec.value("cluster_out", *this->cluster_out);
    if (spec.contains("patterns_out"))
        this->patterns_file_out = spec.value("patterns_out", *this->patterns_file_out);
    this->corpus_file = spec.value("corpus_file", this->corpus_file);
    this->workers = spec.value("workers", this->workers);
    if (spec.contains("corpus_type"))
        this->corpus_type = spec.at("corpus_type").get<CorpusType>();
    this->strict_rex_string_checking = spec.value("strict_rex_string_checking", this->strict_rex_string_checking);
    if (spec.contains("scorer_type"))
        this->scorer_type = spec.at("scorer").get<ScorerType>();
    this->top_k_edges = spec.value("top_k_edges", this->top_k_edges);
    if (spec.contains("existing_graph"))
        this->existing_graph_path = spec.value("existing_graph", *this->existing_graph_path);
    this->wine_path = spec.value("wine_path", this->wine_path);
    this->rex_path = spec.value("rex_path", this->rex_path);
    this->mcl_path = spec.value("mcl_path", this->mcl_path);
}
