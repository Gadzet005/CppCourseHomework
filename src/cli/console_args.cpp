#include <getopt.h>

#include <cli/console_args.hpp>
#include <stdexcept>

using namespace std;

constexpr struct option longOptions[] = {
    {"file", required_argument, nullptr, 'f'},
    {"p-type", required_argument, nullptr, 'p'},
    {"v-type", required_argument, nullptr, 'v'},
    {"v-flow-type", required_argument, nullptr, 'q'},
    {"save-dir", required_argument, nullptr, 's'},
    {"save-rate", required_argument, nullptr, 'r'},
    {"use-save", required_argument, nullptr, 'u'},
    {nullptr, 0, nullptr, 0}};

const char* shortOptions = "f:p:v:q:s:r:u:";

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
            case 's':
                args.saveDir = optarg;
                break;
            case 'r':
                args.saveRate = std::stoi(optarg);
                break;
            case 'u':
                args.saveFile = optarg;
                break;
            case -1:
            default:
                throw std::invalid_argument("Invalid option");
        }
    }

    return args;
}

std::pair<bool, string> ConsoleArgs::validate() {
    if (!filePath.empty() && !saveFile.empty()) {
        return {false,
                "Both --file and --use-save options cannot be provided."};
    }
    if (filePath.empty() && saveFile.empty()) {
        return {false, "--file or --use-save option is required."};
    }
    if (saveRate == 0) {
        return {false, "--save-rate option must be greater than 0."};
    }

    return {true, ""};
}