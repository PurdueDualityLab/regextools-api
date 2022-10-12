//
// Created by charlie on 10/6/22.
//

#include <unordered_set>
#include <string>
#include <re2/re2.h>
#include <egret.h>
#include <iostream>
#include <random>
#include <nlohmann/json.hpp>
#include <condition_variable>
#include <thread>
#include <fstream>
#include <threadpool/ThreadPool.h>
#include <spdlog/spdlog.h>
#include "librereuse/db/pattern_reader.h"

struct regex_exception : public std::exception {
    explicit regex_exception(std::string msg)
    : msg(std::move(msg))
    {
        this->msg = "Regex error: " + std::move(this->msg);
    }

    const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
        return this->msg.c_str();
    }

private:
    std::string msg;
};

struct RegexInfo {

    explicit RegexInfo(std::string pattern) {

        /*
        try {
            this->regex = timed_regex_build(pattern);
        } catch (std::runtime_error &exe) {
            throw regex_exception("compiling regex timed out");
        }*/
        this->regex = std::make_unique<re2::RE2>(pattern);
        if (!this->regex->ok()) {
            throw regex_exception("could not compile regex");
        }

        // Generate strings
        std::vector<std::string> strings;
        try {
            strings = run_engine(pattern, "evil");
            strings.erase(strings.begin());
        } catch (std::runtime_error &exe) {
            std::string msg = "Could not generate egret strings: ";
            msg += exe.what();
            throw regex_exception(msg);
        }

        for (auto &str : strings) {
            if (re2::RE2::FullMatch(str, *this->regex)) {
                this->positive.insert(std::move(str));
            } else {
                this->negative.insert(std::move(str));
            }
        }
    }

    double score(const RegexInfo &other) const {
        // Check this regex against other's strings
        double tp = 0, tn = 0;

        for (const auto &pos : other.positive) {
            if (re2::RE2::FullMatch(pos, *this->regex)) {
                tp++;
            }
        }

        for (const auto &neg : other.negative) {
            if (!re2::RE2::FullMatch(neg, *this->regex)) {
                tn++;
            }
        }

        for (const auto &pos : this->positive) {
            if (re2::RE2::FullMatch(pos, *other.regex)) {
                tp++;
            }
        }

        for (const auto &neg : this->negative) {
            if (!re2::RE2::FullMatch(neg, *other.regex)) {
                tn++;
            }
        }

        return static_cast<double>(tp + tn) / static_cast<double>(this->positive.size() + this->negative.size() + other.positive.size() + other.negative.size());
    }

    std::unique_ptr<re2::RE2> regex;
    std::unordered_set<std::string> positive;
    std::unordered_set<std::string> negative;
};

std::vector<RegexInfo> build_regex_infos(std::vector<std::string> regex_patterns, std::atomic_long &built) {
    std::vector<RegexInfo> regex_infos;
    for (auto &pattern : regex_patterns) {
        spdlog::info("build_regex_infos: Building regex #{} /{}/...", built++, pattern);
        try {
            RegexInfo newInfo(std::move(pattern));
            regex_infos.push_back(std::move(newInfo));
        } catch (regex_exception &exe) {
            // skip this one regex
            spdlog::warn("build_regex_infos: Skipping regex /{}/: {}", pattern, exe.what());
        }
    }

    return regex_infos;
}

std::vector<RegexInfo> build_regex_infos_threaded(std::vector<std::string> regex_patterns, unsigned int workers) {
    ThreadPool work_pool(workers);

    auto chunk_size = regex_patterns.size() / workers;
    std::vector<std::vector<std::string>> chunks;
    auto start_chunk_it = regex_patterns.begin();
    while (start_chunk_it <= regex_patterns.end()) {
        std::vector<std::string> chunk;
        auto end_chunk_it = std::next(start_chunk_it, chunk_size);
        std::move(start_chunk_it, end_chunk_it, std::back_inserter(chunk));
        chunks.push_back(std::move(chunk));
        start_chunk_it = end_chunk_it + 1;
    }

    std::vector<std::future<std::vector<RegexInfo>>> tasks;
    std::atomic_long built(0);
    for (auto &orig_chunk : chunks) {
        auto task = work_pool.enqueue([chunk = std::move(orig_chunk), &built]() {
            return build_regex_infos(std::move(chunk), built);
        });
        tasks.push_back(std::move(task));
    }

    // We have each task
    std::vector<RegexInfo> all_infos;
    for (auto &task : tasks) {
        auto infos = task.get();
        std::move(infos.begin(), infos.end(), std::back_inserter(all_infos));
    }

    return all_infos;
}

std::vector<RegexInfo> cluster(RegexInfo seed, std::vector<RegexInfo> &regexes, int max_cluster_size) {
    unsigned long id = 0;
    for (const auto &regex : regexes) {
        if (!regex.regex) {
            spdlog::warn("cluster: regex at id {} has a null regex", id);
        }
        id++;
    }

    std::vector<std::pair<typename std::vector<RegexInfo>::size_type, double>> scores;

    std::vector<RegexInfo>::size_type idx = 0;
    for (const auto &regexInfo : regexes) {
        auto score = seed.score(regexInfo);
        scores.emplace_back(idx, score);
        idx++;
    }

    // Now that we have a score for every regex, sort the list by the scores and take the top n values
    std::sort(scores.begin(), scores.end(),
              [](std::pair<typename std::vector<RegexInfo>::size_type, double> left, std::pair<typename std::vector<RegexInfo>::size_type, double> right) {
        return left.second > right.second;
    });

    std::vector<RegexInfo> new_cluster;
    new_cluster.push_back(std::move(seed));
    for (int i = 0; i < max_cluster_size; i++) {
        idx = scores[i].first;
        // Steal that particular regex info
        new_cluster.push_back(std::move(regexes[idx]));
    }

    for (auto it = regexes.begin(); it != regexes.end();) {
        if (!it->regex) {
            it = regexes.erase(it);
        } else {
            ++it;
        }
    }

    return new_cluster;
}

RegexInfo randomly_remove_info(std::vector<RegexInfo> &regexes) {
    // gets 'entropy' from device that generates random numbers itself
    // to seed a mersenne twister (pseudo) random generator
    std::mt19937 generator(std::random_device{}());

    // make sure all numbers have an equal chance.
    // range is inclusive (so we need -1 for vector index)
    std::uniform_int_distribution<std::size_t> distribution(0, regexes.size() - 1);

    std::size_t idx = distribution(generator);

    auto info = std::move(regexes[idx]);
    regexes.erase(std::next(regexes.begin(), idx));

    return info;
}

std::vector<std::vector<RegexInfo>> create_clusters(std::vector<RegexInfo> regexes, int max_cluster_size) {
    // First, randomly select a regex
    std::vector<std::vector<RegexInfo>> clusters;
    while (regexes.size() > max_cluster_size) {
        // Randomly remove a regex
        auto seed = randomly_remove_info(regexes);

        spdlog::info("Starting to cluster with /{}/...", seed.regex->pattern());

        // Create a cluster with it
        auto new_cluster = cluster(std::move(seed), regexes, max_cluster_size);
        clusters.push_back(std::move(new_cluster));
    }

    return clusters;
}

int main(int argc, const char **argv) {
    // Read regexes
    if (argc < 3) {
        std::cerr << "usage: cluster-tool <path to cluster> <path to write clusters to>" << std::endl;
        return 1;
    }

    std::string path(argv[1]);
    std::string output_path(argv[2]);
    std::vector<RegexInfo> regex_infos;
    std::atomic_long built(0);
    {
        auto patterns = rereuse::db::read_patterns_from_path(path);
        regex_infos = build_regex_infos(std::move(patterns), built);
        // regex_infos = build_regex_infos_threaded(std::move(patterns), std::thread::hardware_concurrency());
    }

    std::cout << "------------\n";
    std::cout << "Clustering " << regex_infos.size() << " regexes\n";
    std::cout << "-------------" << std::endl;

    auto clusters = create_clusters(std::move(regex_infos), 100);

    nlohmann::json clusters_obj;
    for (const auto &cluster : clusters) {
        nlohmann::json cluster_obj;
        for (const auto &regex : cluster) {
            cluster_obj.push_back(regex.regex->pattern());
        }

        clusters_obj.push_back(cluster_obj);
    }

    std::ofstream output(output_path);
    output << clusters_obj << std::endl;

    return 0;
}
