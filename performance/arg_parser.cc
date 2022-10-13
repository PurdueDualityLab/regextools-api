//
// Created by charlie on 10/10/22.
//

#include "arg_parser.h"

#include <getopt.h>
#include <stdexcept>

const static option program_options[] = {
        { "--help", no_argument, nullptr, 'h' },
        {nullptr, 0, nullptr, 0},
};

ProgramArgs::ProgramArgs(int argc, char **argv)
: showHelp(false) {
    int idx;
    int c;
    while ((c = getopt_long(argc, argv, "h", program_options, &idx)) > 0) {
        switch (c) {
            case 'h':
                this->showHelp = true;
                break;

            default:
                throw std::runtime_error("Unexpected argument");
        }
    }

    for (idx = optind; idx < argc; idx++) {
        this->cluster_file = std::string(argv[idx]);
    }
}
