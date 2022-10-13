//
// Created by charlie on 10/10/22.
//

#ifndef REGEXTOOLS_ARG_PARSER_H
#define REGEXTOOLS_ARG_PARSER_H

#include <string>

struct ProgramArgs {
    ProgramArgs(int argc, char **argv);

    bool showHelp;
    std::string cluster_file;
};

#endif //REGEXTOOLS_ARG_PARSER_H
