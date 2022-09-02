//
// Created by charlie on 8/31/21.
//

#ifndef _CLUSTER_H
#define _CLUSTER_H

#include <string>
#include <unordered_set>
#include <optional>

#include <re2/re2.h>
#include <re2/set.h>

namespace rereuse::db {
    class Cluster {
    public:
        explicit Cluster();
        explicit Cluster(const std::unordered_set<std::string>& patterns);

        bool add_pattern(const std::string &pattern);
        bool compile();
        int get_size() const { return this->size; }
        bool is_compiled() const { return this->set_is_compiled; }
        std::vector<std::string> &get_patterns() { return this->patterns; }

        const RE2::Set &get_regex_set() const;

    private:
        int size;
        bool set_is_compiled;
        std::unique_ptr<re2::RE2::Set> regex_set;
        std::vector<std::string> patterns;
    };
}

#endif //_CLUSTER_H
