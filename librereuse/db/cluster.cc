//
// Created by charlie on 8/31/21.
//
#include "cluster.h"

#include <iostream>

rereuse::db::Cluster::Cluster(const std::unordered_set<std::string> &patterns)
        : size(0),
          set_is_compiled(false) {
    auto opts = RE2::Options();
    // opts.set_log_errors(false);
    opts.set_max_mem(1073741824); // 1 GiB of memory
    this->regex_set = std::make_unique<RE2::Set>(opts, RE2::ANCHOR_BOTH);

    for (const auto &pattern : patterns) {
        std::string error_msg;
        int ret = this->regex_set->Add(pattern, &error_msg);
        if (ret < 0) {
            // std::cerr << "Error while parsing regex /" << pattern << "/: " << error_msg << std::endl;
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
        // TODO Maybe do something about that error msg...
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
