//
// Created by charlie on 1/25/23.
//

#include <iostream>
#include <thread>

#include "nlohmann/json.hpp"

#include "rex_wrapper.h"
#include "librereuse/db/pattern_reader.h"
#include "librereuse/util/chunk.h"
#include "ThreadPool.h"

#define WINE_PATH "/usr/bin/wine"
#define REX_PATH "/home/cmsale/Rex.exe"

static constexpr std::string_view wine_path(WINE_PATH);
static constexpr std::string_view rex_path(REX_PATH);

int main(int argc, char **argv) {

    if (argc < 3) {
        std::cerr << "usage: tool <input path to objects> <path to strings output>" << std::endl;
        return 1;
    }

    // Read in all the regexes
    auto input_regexes = rereuse::db::read_patterns_from_path(argv[1]);

    // chunk everything
    auto input_chunks = rereuse::util::chunk(std::move(input_regexes), std::thread::hardware_concurrency());

    // Setup up rex
    RexWrapper wrapper(rex_path.data(), wine_path.data());
    ThreadPool thread_pool(std::thread::hardware_concurrency());

    std::vector<std::future<std::unordered_map<std::string, std::vector<std::string>>>> tasks;
    for (auto &chunk : input_chunks) {
        auto task = thread_pool.enqueue([&wrapper, regexes = std::move(chunk)] {
            std::unordered_map<std::string, std::vector<std::string>> rex_strings;
            for (const auto &regex : regexes) {
                try {
                    auto strings = wrapper.generate_strings(regex, 100);
                    auto pair = std::make_pair(regex, std::move(strings));
                    rex_strings.insert(std::move(pair));
                } catch (std::runtime_error &exe) {
                    continue;
                }
            }

            return rex_strings;
        });
        tasks.push_back(std::move(task));
    }

    // merge all the tasks into one map
    std::unordered_map<std::string, std::vector<std::string>> all_strings;
    for (auto &task : tasks) {
        auto strings = task.get();
        all_strings.merge(std::move(strings));
    }

    // Serialize and output
    nlohmann::json output_obj = all_strings;

    std::ofstream output(argv[2]);
    output << output_obj << std::endl;

    // DONE

    return 0;
}
