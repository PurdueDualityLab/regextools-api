//
// Created by charlie on 12/3/22.
//

#include "string_file_registry.h"

#include <fstream>
#include "nlohmann/json.hpp"

StringFileRegistry::StringFileRegistry() {
    // Figure out where we are going to put the strings
    auto temp_dir_parent = std::filesystem::temp_directory_path();
    this->strings_directory_path = temp_dir_parent.append("rex_strings");
}

StringFileRegistry::~StringFileRegistry() {
    // Delete all strings files when this object gets cleaned up
    std::filesystem::remove_all(this->strings_directory_path);
}

void StringFileRegistry::store_strings(unsigned long regex_id, const std::vector<std::string>& strings) {
    if (this->string_files.count(regex_id) == 0) {
        // We need to create a new path for this one
        auto new_file_path = this->strings_directory_path / ("rex_strings_" + std::to_string(regex_id));
        this->string_files[regex_id] = new_file_path;
    }

    // serialize the strings as JSON
    nlohmann::json strings_obj = strings;

    // Write the strings to the given file
    std::ofstream strings_writer(this->string_files[regex_id].string());
    strings_writer << strings_obj;
}

std::vector<std::string> StringFileRegistry::retrieve_strings(unsigned long id) {
    // Read the file
    auto existing_path = this->string_files.at(id);
    std::ifstream input(existing_path.string());
    nlohmann::json strings;
    input >> strings;

    // Parse strings
    return strings.get<std::vector<std::string>>();
}
