//
// Created by charlie on 8/31/21.
//
#include <random>
#include "cluster.h"

#include "spdlog/spdlog.h"
#include "egret/egret.h" // TODO fix this in egret

rereuse::db::Cluster::Cluster(const std::vector<RegexEntity>& input_entities)
        : size(0),
          set_is_compiled(false) {
    auto opts = RE2::Options();
    // opts.set_log_errors(false);
    auto max_mem_size = static_cast<int64_t>(1073741824) * 2; // 1GiB * num of GiBs
    opts.set_max_mem(max_mem_size); // 1 GiB of memory
    this->regex_set = std::make_unique<RE2::Set>(opts, RE2::ANCHOR_BOTH);

    for (const auto &entity : input_entities) {
        std::string error_msg;
        int ret = this->regex_set->Add(entity.get_pattern(), &error_msg);
        if (ret < 0) {
            spdlog::error("Cluster::Cluster: Error while parsing regex /{}/: {}", entity.get_pattern(), error_msg);
        } else {
            this->size++; // A new regex was added, so increment the size
            this->entities.push_back(entity);
        }
    }
}

rereuse::db::Cluster::Cluster()
        : size(0),
          set_is_compiled(false) {
    auto opts = RE2::Options();
    // opts.set_log_errors(false);
    auto max_mem_size = static_cast<int64_t>(1073741824) * 2; // 1GiB * num of GiBs
    opts.set_max_mem(max_mem_size); // 1 GiB of memory
    this->regex_set = std::make_unique<RE2::Set>(opts, RE2::ANCHOR_BOTH);
}

bool rereuse::db::Cluster::add_entity(const RegexEntity &entity) {
    std::string error_msg;
    int ret = this->regex_set->Add(entity.get_pattern(), &error_msg);
    if (ret >= 0) {
        // Successfully added the regex
        this->set_is_compiled = false; // This becomes out of date...
        this->size++;
        this->entities.push_back(entity);
        return true;
    } else {
        spdlog::error("Cluster:add_entity: Could not add regex /{}/: {}", entity.get_pattern(), error_msg);
        return false;
    }
}

bool rereuse::db::Cluster::compile(bool eager) {
    if (!this->set_is_compiled) {
        this->set_is_compiled = this->regex_set->Compile(eager);
    }
    return this->set_is_compiled;
}

// Return a reference to the owned object (not sure if ths is good...)
const RE2::Set &rereuse::db::Cluster::get_regex_set() const {
    return *this->regex_set;
}

static std::unordered_set<std::string> cull_string_set(std::unordered_set<std::string> &&strings, std::size_t end_size) {
    if (end_size > strings.size()) {
        return strings;
    }

    std::vector<std::string> input_strings;
    std::move(strings.begin(), strings.end(), std::back_inserter(input_strings));
    std::unordered_set<std::string> culled_strings;

    // Choose end_size number of indices to take
    auto randomizer = std::mt19937(std::random_device()());
    std::uniform_int_distribution<std::mt19937::result_type> to_remove_dist(0, input_strings.size() - 1);
    std::vector<std::size_t> selected_strings(end_size);
    for (auto &selected_string_idx : selected_strings) {
        selected_string_idx = to_remove_dist(randomizer);
    }

    for (const auto &idx : selected_strings) {
        culled_strings.insert(std::move(input_strings[idx]));
    }

    return culled_strings;
}

void rereuse::db::Cluster::prime() {
    if (!this->is_compiled()) {
        spdlog::error("Cluster must be compiled before priming");
        return;
    }

    // Generate a set of strings for this cluster
    std::unordered_set<std::string> priming_strings;
    for (const auto &entity : this->entities) {
        auto strings = run_engine(entity.get_pattern(), "regexes");
        re2::RE2 pattern_regex(entity.get_pattern());
        std::unordered_set<std::string> positive_strings;
        //std::copy_if(strings.begin(), strings.end(), std::inserter(positive_strings, positive_strings.begin()), [&pattern_regex](std::string &str) { return re2::RE2::FullMatch(str, pattern_regex); });
        std::copy_if(strings.begin(), strings.end(), std::inserter(positive_strings, positive_strings.begin()), [&pattern_regex](std::string &str) { return true; });

        positive_strings = cull_string_set(std::move(positive_strings), 7);

        std::move(positive_strings.begin(), positive_strings.end(), std::inserter(priming_strings, priming_strings.begin()));
    }

    if constexpr (true) {
        priming_strings = cull_string_set(std::move(priming_strings), priming_strings.size() >> 1);
    }

    // We have a set of strings that should cover the cluster. Run them all to "prime" this cluster
    unsigned long id = 0;
    spdlog::info("Testing {} strings", priming_strings.size());
    for (const auto &priming_string : priming_strings) {
        std::vector<int> indices;
        this->regex_set->Match(priming_string, &indices);
        // spdlog::info("Tested string {}/{}", ++id, priming_strings.size());
    }

    // Primed
}
