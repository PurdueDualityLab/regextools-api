//
// Created by charlie on 11/14/22.
//

#include "malloc_info_parser.h"

#include <array>
#include <memory>
#include <malloc.h>
#include <stdexcept>
#include <iostream>

#include "pugixml/src/pugixml.hpp"

std::size_t parse_malloc_info(std::string &&buffer) {
    pugi::xml_document document;
    pugi::xml_parse_result result = document.load_buffer_inplace(buffer.data(), buffer.size());
    if (!result) {
        throw std::runtime_error("Could not parse malloc_info xml");
    }

    auto aspace_node_set = document.select_nodes("/malloc/aspace[@type = 'total']");
    if (!aspace_node_set.empty()) {
        auto total_node = aspace_node_set[0];
        std::size_t total_size_bytes = total_node.node().attribute("size").as_ullong(0);
        return total_size_bytes;
    } else {
        throw std::runtime_error("Could not retrieve total aspace node from XML");
    }
}

std::size_t heap_get_allocated_bytes() {
    char *raw_buffer = nullptr;
    std::size_t raw_buffer_size;
    FILE *stream = open_memstream(&raw_buffer, &raw_buffer_size);

    int ret = malloc_info(0, stream);
    if (ret) {
        throw std::runtime_error("Error while running malloc_info");
    }

    fclose(stream);

    std::string buffer(raw_buffer, raw_buffer_size);

    // parse the buffer
    auto total_size = parse_malloc_info(std::move(buffer));
    return total_size;
}
