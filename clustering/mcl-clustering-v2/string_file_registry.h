//
// Created by charlie on 12/3/22.
//

#ifndef REGEXTOOLS_STRING_FILE_REGISTRY_H
#define REGEXTOOLS_STRING_FILE_REGISTRY_H

#include <string>
#include <unordered_map>
#include <filesystem>
#include <vector>

/**
 * System for keeping track of all of the strings for all of the regexes. Store strings for a regex in here
 * and then retrieve them from here when you need them again
 */
class StringFileRegistry {
public:
    StringFileRegistry();
    ~StringFileRegistry();

    void store_strings(unsigned long regex_id, const std::vector<std::string>& strings);
    std::vector<std::string> retrieve_strings(unsigned long id);

private:
    std::filesystem::path strings_directory_path;  // path containing all the string files
    std::unordered_map<unsigned long, std::filesystem::path> string_files;  // Map of paths. Key is the regex id
};


#endif //REGEXTOOLS_STRING_FILE_REGISTRY_H
