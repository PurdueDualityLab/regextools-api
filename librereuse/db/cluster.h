//
// Created by charlie on 8/31/21.
//

#ifndef _CLUSTER_H
#define _CLUSTER_H

#include <string>
#include <unordered_set>
#include <optional>

#include "re2/re2.h"
#include "re2/set.h"

#include "regex_entity.h"

namespace rereuse::db {
    class Cluster {
    public:
        explicit Cluster();
        explicit Cluster(const std::vector<RegexEntity>& entities);

        bool add_entity(const RegexEntity &entity);
        bool compile(bool eager = false);
        int get_size() const { return this->size; }
        bool is_compiled() const { return this->set_is_compiled; }
        std::vector<RegexEntity> &get_entities() { return this->entities; }
        /**
         * Primes the set cache for this specific cluster, improving performance
         */
        void prime();

        const RE2::Set &get_regex_set() const;

    private:
        int size;
        bool set_is_compiled;
        std::unique_ptr<re2::RE2::Set> regex_set;
        std::vector<RegexEntity> entities;
    };
}

#endif //_CLUSTER_H
