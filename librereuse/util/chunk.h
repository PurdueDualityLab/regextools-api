//
// Created by charlie on 1/25/23.
//

#ifndef REGEXTOOLS_CHUNK_H
#define REGEXTOOLS_CHUNK_H

#include <vector>

namespace rereuse::util {

    /**
     * Chunks a vector
     * @tparam ElementTp The elements found in vector
     * @param elements The vector elements to chunk
     * @param divide_into How many chunks to make
     * @return List of chunks
     */
    template<typename ElementTp>
    auto chunk(std::vector<ElementTp> elements, unsigned int divide_into) -> std::vector<std::vector<ElementTp>> {
        if (divide_into == 0) {
            return {};
        } else if (divide_into == 1) {
            return std::vector<std::vector<ElementTp>> {std::move(elements)};
        }

        auto items_per_chunk = (elements.size() / divide_into);
        std::vector<std::vector<ElementTp>> chunks;
        for (int chunk_index = 0; chunk_index < divide_into; chunk_index++) {
            auto offset = chunk_index * items_per_chunk;
            auto begin_iter = std::next(elements.begin(), offset);
            auto end_iter = (std::distance(begin_iter, elements.end()) < items_per_chunk) ? elements.end() : std::next(
                    begin_iter, items_per_chunk);
            std::vector<ElementTp> chunk;
            std::move(begin_iter, end_iter, std::back_inserter(chunk));
            chunks.push_back(std::move(chunk));
        }

        return chunks;
    }

}

#endif //REGEXTOOLS_CHUNK_H
