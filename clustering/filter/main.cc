//
// Created by charlie on 10/10/22.
//

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <egret.h>
#include "arg_parser.h"
#include "librereuse/db/pattern_reader.h"

std::optional<std::string> read_next_pattern(std::istream &input_stream) {
    while (!input_stream.eof()) {
        std::string next_line;
        std::getline(input_stream, next_line);

        // Next, try to parse the pattern
        try {
            auto obj = nlohmann::json::parse(next_line);
            auto pattern = obj.at("pattern").get<std::string>();
            // We found a valid pattern
            return {pattern};
        } catch (nlohmann::json::parse_error &exe) {
            continue;
        } catch (nlohmann::json::out_of_range &exe) {
            continue;
        }
    }

    // If we made it to the end of the file, there is no next pattern
    return {};
}

bool contains_only_ascii(const std::string &input_string) {
    for (const auto ch : input_string) {
        if (static_cast<unsigned char>(ch) > 127) {
            return false;
        }
    }

    return true;
}

void output_regexes(std::vector<std::string> patterns, std::ostream &os) {
    for (auto &pattern : patterns) {
        nlohmann::json obj;
        obj["pattern"] = std::move(pattern);
        os << to_string(obj) << '\n';
    }
    os << std::endl;
}

int main(int argc, char **argv) {

    ProgramArguments args;
    try {
        args = ProgramArguments(argc, argv);
    } catch (std::runtime_error &exe) {
        std::cerr << "Error while parsing arguments: " << exe.what() << std::endl;
        return 1;
    }

    if (args.showHelp) {
        std::cout << "Help!" << std::endl;
        return 0;
    }

    std::ifstream input_file(args.input);
    if (!input_file.is_open()) {
        std::cerr << "Could not open input file" << std::endl;
        return 1;
    }

    std::vector<std::string> valid_regexes;
    std::optional<std::string> next_regex;
    re2::RE2::Options re2_opts;
    re2_opts.set_log_errors(false);
    while ((next_regex = read_next_pattern(input_file)).has_value()) {
        std::string next_pattern = next_regex.value();

        // Actual filter it
        std::cout << "Checking regex /" << next_pattern << "/..." << std::endl;

        if (args.max_length > 0) {
            std::cout << "Checking against max length... " << std::flush;
            if (next_pattern.length() > args.max_length) {
                std::cout << "FAILED" << std::endl;
                continue;
            }

            std::cout << "PASSED" << std::endl;
        }

        if (args.asciiOnly) {
            std::cout << "Checking against ascii only... " << std::flush;
            if (!contains_only_ascii(next_pattern)) {
                std::cout << "FAILED" << std::endl;
                continue;
            }
            std::cout << "PASSED" << std::endl;
        }

        if (args.doRE2Filter) {
            { // Putting this in a scope to ensure deletion??
                std::cout << "Checking against re2... " << std::flush;
                re2::RE2 newRegex(next_pattern, re2_opts);
                if (!newRegex.ok()) {
                    std::cout << "FAILED" << std::endl;
                    continue; // Found a point where this isn't valid, so continue onto the next one
                } else {
                    std::cout << "PASSED" << std::endl;
                }
            }
        }

        if (args.doEgretFilter) {
            std::cout << "Checking against egret... " << std::flush;
            try {
                run_engine(next_pattern, "evil");
            } catch (std::runtime_error &err) {
                // Error while running egret
                std::cout << "FAILED" << std::endl;
                continue;
            }
            std::cout << "PASSED" << std::endl;
        }

        std::cout << "Regex is valid" << std::endl;
        // At this point, we made it through the filters, so this pattern is valid
        valid_regexes.push_back(std::move(next_pattern));
    }

    // Figure out where to output to
    auto regex_count = valid_regexes.size();
    if (!args.output.empty()) {
        std::ofstream os(args.output);
        if (os.is_open()) {
            output_regexes(std::move(valid_regexes), os);
        } else {
            std::cerr << "Could not open output file" << std::endl;
        }
    } else {
        output_regexes(std::move(valid_regexes), std::cout);
    }

    // We now have a bunch of valid regexes. Do something
    std::cout << "Found " << regex_count << " valid regex(es)" << std::endl;
    return 0;
}
