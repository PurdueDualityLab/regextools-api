//
// Created by charlie on 1/17/23.
//

#ifndef REGEXTOOLS_REGEX_ENTITY_H
#define REGEXTOOLS_REGEX_ENTITY_H

#include <string>

namespace rereuse::db {

    /**
     * Regex entity that holds a regex pattern, where to find it on the internet, its license, and more
     */
    class RegexEntity {
    public:
        RegexEntity() = default;
        RegexEntity(const RegexEntity &) = default;
        RegexEntity(RegexEntity &&) = default;
        RegexEntity &operator=(const RegexEntity &) = default;
        RegexEntity &operator=(RegexEntity &&) = default;

        RegexEntity(std::string pattern, std::string repo_location, std::string file_path,
                    size_t line_number, std::string license);

        ~RegexEntity() = default;

        const std::string &get_pattern() const {
            return pattern;
        }

        const std::string &get_repo_location() const {
            return repo_location;
        }

        const std::string &get_file_path() const {
            return file_path;
        }

        size_t get_line_number() const {
            return line_number;
        }

        const std::string &get_license() const {
            return license;
        }

    private:
        std::string pattern; // The actual regex pattern
        std::string repo_location; // URL to hosting location of repository
        std::string file_path; // Relative path to the file containing this regex
        std::size_t line_number; // The line number of this regex
        std::string license; // How this regex is licensed
    };
} // db

#endif //REGEXTOOLS_REGEX_ENTITY_H
