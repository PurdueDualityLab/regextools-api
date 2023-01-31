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

        RegexEntity(std::string pattern, std::string id);

        ~RegexEntity() = default;

        const std::string &get_pattern() const {
            return pattern;
        }

        const std::string &get_id() const {
            return id;
        }
    private:
        std::string pattern; // The actual regex pattern
        std::string id; // corresponding entity id
    };
} // db

#endif //REGEXTOOLS_REGEX_ENTITY_H
