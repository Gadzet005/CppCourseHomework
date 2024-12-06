#include <getopt.h>

#include <cli/console_args_parser.hpp>
#include <stdexcept>

constexpr struct option longOptions[] = {
    {"file", required_argument, nullptr, 'f'},
    {"p-type", required_argument, nullptr, 'p'},
    {"velocity-type", required_argument, nullptr, 'v'},
    {"velocity-flow-type", required_argument, nullptr, 'q'},
    {nullptr, 0, nullptr, 0}};

const char* shortOptions = "f:p:v:q:";

ConsoleArgs parseConsoleArguments(int argc, char* argv[]) {
    ConsoleArgs args;

    int c;
    while ((c = getopt_long(argc, argv, shortOptions, longOptions, NULL)) !=
           -1) {
        switch (c) {
            case 'f':
                args.filePath = optarg;
                break;
            case 'p':
                args.pType = parseType(optarg);
                break;
            case 'v':
                args.velocityType = parseType(optarg);
                break;
            case 'q':
                args.velocityFlowType = parseType(optarg);
                break;
            case -1:
            default:
                throw invalid_argument("Invalid option");
        }
    }

    return args;
}