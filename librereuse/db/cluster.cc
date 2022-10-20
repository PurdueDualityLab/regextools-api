//
// Created by charlie on 8/31/21.
//
#include "cluster.h"

#include "spdlog/spdlog.h"
#include "egret.h"

rereuse::db::Cluster::Cluster(const std::unordered_set<std::string> &patterns)
        : size(0),
          set_is_compiled(false) {
    auto opts = RE2::Options();
    // opts.set_log_errors(false);
    auto max_mem_size = static_cast<int64_t>(1073741824) * 2; // 1GiB * num of GiBs
    opts.set_max_mem(max_mem_size); // 1 GiB of memory
    this->regex_set = std::make_unique<RE2::Set>(opts, RE2::ANCHOR_BOTH);

    for (const auto &pattern : patterns) {
        std::string error_msg;
        int ret = this->regex_set->Add(pattern, &error_msg);
        if (ret < 0) {
            spdlog::error("Cluster::Cluster: Error while parsing regex /{}/: {}", pattern, error_msg);
        } else {
            this->size++; // A new regex was added, so increment the size
            this->patterns.push_back(pattern);
        }
    }
}

rereuse::db::Cluster::Cluster()
        : size(0),
          set_is_compiled(false) {
    auto opts = RE2::Options();
    // opts.set_log_errors(false);
    this->regex_set = std::make_unique<RE2::Set>(opts, RE2::ANCHOR_BOTH);
}

bool rereuse::db::Cluster::add_pattern(const std::string &pattern) {
    std::string error_msg;
    int ret = this->regex_set->Add(pattern, &error_msg);
    if (ret >= 0) {
        // Successfully added the regex
        this->set_is_compiled = false; // This becomes out of date...
        this->size++;
        this->patterns.push_back(pattern);
        return true;
    } else {
        spdlog::error("Cluster:add_pattern: Could not add regex /{}/: {}", pattern, error_msg);
        return false;
    }
}

bool rereuse::db::Cluster::compile() {
    if (!this->set_is_compiled) {
        this->set_is_compiled = this->regex_set->Compile();
    }
    return this->set_is_compiled;
}

// Return a reference to the owned object (not sure if ths is good...)
const RE2::Set &rereuse::db::Cluster::get_regex_set() const {
    return *this->regex_set;
}

void rereuse::db::Cluster::prime() {
    if (!this->is_compiled()) {
        spdlog::error("Cluster must be compiled before priming");
        return;
    }

    // Generate a set of strings for this cluster
    std::unordered_set<std::string> priming_strings;
    for (const auto &pattern : this->patterns) {
        auto strings = run_engine(pattern, "regexes");
        std::move(strings.begin(), strings.end(), std::inserter(priming_strings, priming_strings.begin()));
    }

    // We have a set of strings that should cover the cluster. Run them all to "prime" this cluster
    for (const auto &priming_string : priming_strings) {
        std::vector<int> indices;
        this->regex_set->Match(priming_string, &indices);
    }

    // Primed
}
