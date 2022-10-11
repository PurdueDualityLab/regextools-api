//
// Created by charlie on 10/6/22.
//

#include <unordered_set>
#include <string>
#include <memory>
#include <sstream>
#include <re2/re2.h>
#include <egret.h>
#include <iostream>
#include <random>
#include <nlohmann/json.hpp>
#include <condition_variable>
#include <thread>
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

std::unique_ptr<re2::RE2> timed_regex_build(const std::string &pattern) {
    using namespace std::chrono_literals;

    std::mutex mut;
    std::condition_variable cv;
    std::unique_ptr<re2::RE2> resulting_regex;

    std::thread th([&cv, &resulting_regex, &pattern] {
        resulting_regex = std::make_unique<re2::RE2>(pattern);
        cv.notify_one();
    });

    th.detach();

    {
        std::unique_lock lock(mut);
        if (cv.wait_for(lock, 3s) == std::cv_status::timeout) {
            throw std::runtime_error("building regex timed out");
        }
    }

    return resulting_regex;
}

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

        return (tp + tn) / static_cast<double>(this->positive.size() + this->negative.size() + other.positive.size() + other.negative.size());
    }

    std::unique_ptr<re2::RE2> regex;
    std::unordered_set<std::string> positive;
    std::unordered_set<std::string> negative;
};

std::vector<RegexInfo> build_regex_infos(std::vector<std::string> regex_patterns) {
    std::vector<RegexInfo> regex_infos;
    unsigned long built = 1;
    for (auto &pattern : regex_patterns) {
        std::cout << "Building regex #" << built++ << " /" << pattern << "/..." << std::endl;
        try {
            RegexInfo newInfo(std::move(pattern));
            regex_infos.push_back(std::move(newInfo));
        } catch (regex_exception &exe) {
            // skip this one regex
            std::cerr << "Skipping regex /" << pattern << "/: " << exe.what() << std::endl;
        }
    }

    return regex_infos;
}

std::vector<RegexInfo> cluster(RegexInfo seed, std::vector<RegexInfo> &regexes, int max_cluster_size) {
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
    std::vector<unsigned long> idx_to_remove;
    new_cluster.push_back(std::move(seed));
    for (int i = 0; i < max_cluster_size; i++) {
        auto idx = scores[i].first;
        // Steal that particular regex info
        new_cluster.push_back(std::move(regexes[idx]));
        // Keep track of which idxes to erase in reverse order
        idx_to_remove.insert(idx_to_remove.begin(), idx);
    }

    // Erase all the references
    for (const auto &removeIdx : idx_to_remove) {
        regexes.erase(regexes.begin() + removeIdx);
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
    regexes.erase(regexes.begin() + idx);

    return info;
}

std::vector<std::vector<RegexInfo>> create_clusters(std::vector<RegexInfo> regexes, int max_cluster_size) {
    // First, randomly select a regex
    std::vector<std::vector<RegexInfo>> clusters;
    while (regexes.size() > max_cluster_size) {
        // Randomly remove a regex
        auto seed = randomly_remove_info(regexes);

        // Create a cluster with it
        auto new_cluster = cluster(std::move(seed), regexes, max_cluster_size);
        clusters.push_back(std::move(new_cluster));
    }

    return clusters;
}

int main(int argc, const char **argv) {
    // Read regexes
    if (argc < 2) {
        std::cerr << "usage: clustering <file to cluster>" << std::endl;
        return 1;
    }

    std::string path(argv[1]);
    std::vector<RegexInfo> regexInfos;
    {
        auto patterns = rereuse::db::read_patterns_from_path(path);
        regexInfos = build_regex_infos(std::move(patterns));
    }

    std::cout << "------------\n";
    std::cout << "Clustering " << regexInfos.size() << " regexes\n";
    std::cout << "-------------" << std::endl;

    auto clusters = create_clusters(std::move(regexInfos), 100);

    nlohmann::json clusters_obj;
    for (const auto &cluster : clusters) {
        nlohmann::json cluster_obj;
        for (const auto &regex : cluster) {
            cluster_obj.push_back(regex.regex->pattern());
        }

        clusters_obj.push_back(cluster_obj);
    }

    std::cout << clusters_obj.dump(2, ' ') << std::endl;

    return 0;
}
