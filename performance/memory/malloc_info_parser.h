//
// Created by charlie on 11/14/22.
//

#ifndef REGEXTOOLS_MALLOC_INFO_PARSER_H
#define REGEXTOOLS_MALLOC_INFO_PARSER_H

#include <cstddef>

/**
 * Gets the number of bytes currently free in the heap
 * @return
 */
std::size_t heap_get_allocated_bytes();

#endif //REGEXTOOLS_MALLOC_INFO_PARSER_H
