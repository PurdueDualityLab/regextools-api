//
// Created by charlie on 2/9/22.
//

#ifndef _REX_WRAPPER_H
#define _REX_WRAPPER_H

#include <string>
#include <utility>
#include <vector>
#include <fstream>
#include <iostream>

class TempRegexFile {
public:
    explicit TempRegexFile(const std::string &regex)
    : path("/tmp/" + std::to_string(random()) + "regex-file.json")
    , file(this->path)
    {
        this->file << regex << std::endl;
    }

    ~TempRegexFile() {
        // Close the file
        if (this->file.is_open())
            this->file.close();

        // Remove it
        int ret = remove(this->path.c_str());
        if (ret == 0)
            std::cout << "deleted successfully" << std::endl;
        else
            std::cerr << "Failed to delete temp file" << std::endl;
    }

    const std::string &get_path() const {
        return path;
    }

private:
    std::string path;
    std::ofstream file;
};

class RexWrapper {
public:
    RexWrapper(std::string rex_path, std::string wine_path)
    : rex_path(std::move(rex_path))
    , wine_path(std::move(wine_path))
    { }

    std::vector<std::string> generate_strings(const std::string &pattern, unsigned int count) const;

private:
    std::string rex_path;
    std::string wine_path;
};


#endif //_REX_WRAPPER_H
