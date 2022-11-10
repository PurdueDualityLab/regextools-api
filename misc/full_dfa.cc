//
// Created by charlie on 10/28/22.
//

#include <vector>
#include <string>
#include <iostream>
#include <chrono>

#include "re2/re2.h"
#include "re2/prog.h"
#include "re2/regexp.h"
#include "re2/set.h"

int main() {

    std::vector<std::string> patterns = {
            "[abc]+",
            "[a-z]+",
            "(abc)+",
            "[a-zA-Z0-9]*"
    };

    re2::RE2::Options opts;
    re2::RE2::Set lazy_set(opts, re2::RE2::ANCHOR_BOTH);
    re2::RE2::Set eager_set(opts, re2::RE2::ANCHOR_BOTH);

    for (const auto &pattern : patterns) {
        lazy_set.Add(pattern, nullptr);
        eager_set.Add(pattern, nullptr);
    }

    bool compiled = lazy_set.Compile();
    if (!compiled) {
        std::cerr << "Could not compile lazy set" << std::endl;
        return 1;
    }
    compiled = eager_set.Compile(true);
    if (!compiled) {
        std::cerr << "Could not compile eager set" << std::endl;
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();
    lazy_set.Match("abcbcbabcbacbcbcbabcbaaabcbcbabcbab", nullptr);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Lazy took " << duration.count() << "ns" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    eager_set.Match("abcbcbabcbacbcbcbabcbaaabcbcbabcbab", nullptr);
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Eager took " << duration.count() << "ns" << std::endl;

    return 0;
}

