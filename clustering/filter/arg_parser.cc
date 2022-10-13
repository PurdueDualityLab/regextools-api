//
// Created by charlie on 10/10/22.
//

#include "arg_parser.h"

#include <getopt.h>
#include <stdexcept>

static const option PROGRAM_OPTIONS[] = {
        { "--re2", no_argument, nullptr, 'r' },
        { "--egret", no_argument, nullptr, 'e' },
        { "--max-length", required_argument, nullptr, 'm' },
        { "--output", required_argument, nullptr, 'o' },
        { "--help", no_argument, nullptr, 'h' },
        {nullptr, 0, nullptr, 0}
};

ProgramArguments::ProgramArguments(int argc, char **argv)
: showHelp(false)
, doRE2Filter(false)
, doEgretFilter(false)
, max_length(0)
, asciiOnly(true) {
    int idx;
    int c;
    while ((c = getopt_long(argc, argv, "reho:m:", PROGRAM_OPTIONS, &idx)) > 0) {
        switch (c) {
            case 'r':
                this->doRE2Filter = true;
                break;

            case 'e':
                this->doEgretFilter = true;
                break;

            case 'h':
                this->showHelp = true;
                break;

            case 'o':
                this->output = std::string(optarg);
                break;

            case 'm':
                this->max_length = std::atoll(optarg);
                break;

            default:
                throw std::runtime_error("Unexpected argument");
        }
    }

    for (idx = optind; idx < argc; idx++) {
        this->input = std::string(argv[idx]);
    }
}

ProgramArguments::ProgramArguments()
: showHelp(false)
, doRE2Filter(false)
, doEgretFilter(false)
, output()
, max_length(0)
, asciiOnly(true)
{
}
