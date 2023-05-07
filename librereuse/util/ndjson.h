//
// Created by charlie on 4/4/23.
//

#ifndef REGEXTOOLS_NDJSON_H
#define REGEXTOOLS_NDJSON_H

#include <istream>
#include <vector>
#include "single_include/nlohmann/json.hpp"

namespace rereuse::util {

    template <class EntityTp>
    std::vector<EntityTp> read_ndjson(std::istream &input_stream) {
        std::vector<EntityTp> entities;

        std::string line;
        while (std::getline(input_stream, line)) {
            auto line_obj = nlohmann::json::parse(line);
            auto obj = line_obj.get<EntityTp>();
            entities.push_back(std::move(obj));
        }

        return entities;
    }
}

#endif //REGEXTOOLS_NDJSON_H