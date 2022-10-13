//
// Created by charlie on 10/10/22.
//

#ifndef REGEXTOOLS_ARG_PARSER_H
#define REGEXTOOLS_ARG_PARSER_H

#include <string>

struct ProgramArguments {
    bool showHelp;
    bool doRE2Filter;
    bool doEgretFilter;
    std::string output;
    std::string input;

    std::size_t max_length;
    bool asciiOnly;

    ProgramArguments();
    ProgramArguments(int argc, char **argv);
};

#endif //REGEXTOOLS_ARG_PARSER_H
