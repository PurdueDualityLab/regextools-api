//
// Created by charlie on 1/25/23.
//

#ifndef REGEXTOOLS_REGEX_ENTITY_H
#define REGEXTOOLS_REGEX_ENTITY_H

#include <vector>
#include <string>
#include <memory>
#include "re2/re2.h"

class RegexEntity {
public:
    RegexEntity(const std::string& regex, const std::vector<std::string> &strings);

    /**
     * Get a reference to the underlying regex
     * @return
     */
    const std::unique_ptr<re2::RE2>& get_regex() const {
        return regex;
    }

    /**
     * Get a reference to the strings managed by this entity
     * @return
     */
    const std::vector<std::string> &get_strings() const {
        return strings;
    }

private:
    std::unique_ptr<re2::RE2> regex;
    std::vector<std::string> strings;
};


#endif //REGEXTOOLS_REGEX_ENTITY_H
