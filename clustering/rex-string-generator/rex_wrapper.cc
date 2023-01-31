//
// Created by charlie on 2/9/22.
//

#include "rex_wrapper.h"

#include <cstdio>
#include <unistd.h>
#include <sstream>
#include <algorithm>
#include <memory>
#include <iostream>
#include <functional>

std::vector<std::string> RexWrapper::generate_strings(const std::string &pattern, unsigned int count) const {
    // Generate the commandline
    // Temporary file that holds a single regex. This file will be deleted when the object passes out of scope
    TempRegexFile pattern_regex_file(pattern);
    std::vector<std::string> cmd_fragments = {this->wine_path, this->rex_path, "/e:ASCII", "/k:" + std::to_string(count), "/r:" + pattern_regex_file.get_path()};
    std::stringstream cmd;
    for (auto it = cmd_fragments.begin(); it != cmd_fragments.end() - 1; ++it) {
        cmd << *it << " ";
    }
    cmd << *(cmd_fragments.end() - 1);

    std::array<char, 256> buffer {};
    std::stringstream rex_output_buffer;
    bool failed = false;
    auto pclose_wrapper = [&failed](FILE *pipe_ptr) {
        int ret = pclose(pipe_ptr);
        if (ret != 0)
            failed = true;
    };

    {
        std::unique_ptr<FILE, std::function<void(FILE*)>> pipe(popen(cmd.str().c_str(), "r"), pclose_wrapper);
        if (!pipe) {
            throw std::runtime_error("Could not spawn rex process");
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            rex_output_buffer << buffer.data();
        }
    }

    if (failed)
        throw std::runtime_error("Rex process failed");

    // Parse the output buffer into lines
    std::string rex_strings_buf = rex_output_buffer.str();
    std::vector<std::string> rex_strings_raw;
    size_t pos;
    std::string token;
    while ((pos = rex_strings_buf.find('\n')) != std::string::npos) {
        token = rex_strings_buf.substr(0, pos);
        rex_strings_raw.push_back(std::move(token));
        rex_strings_buf.erase(0, pos + 1);
    }

    // Remove the quotes around each string
    std::vector<std::string> rex_strings;
    std::transform(rex_strings_raw.cbegin(),  rex_strings_raw.cend(), std::back_inserter(rex_strings),
                   [](const std::string &str) -> std::string { return str.substr(1, str.length() - 3); });

    if (rex_strings.empty())
        throw std::runtime_error("Rex strings is empty");

    return rex_strings;
}
