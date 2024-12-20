#include <getopt.h>

#include <cli/console_args.hpp>
#include <stdexcept>

using namespace std;

// clang-format off
constexpr struct option longOptions[] = {
    {"input",           required_argument, nullptr, 'i'},
    {"p-type",          required_argument, nullptr, 'p'},
    {"v-type",          required_argument, nullptr, 'v'},
    {"v-flow-type",     required_argument, nullptr, 'f'},
    {"save",            required_argument, nullptr, 's'},
    {"save-dir",        required_argument, nullptr, 'd'},
    {"save-rate",       required_argument, nullptr, 'r'},
    {"max-iterations",  required_argument, nullptr, 'm'},
    {"threads",         required_argument, nullptr, 't'},
    {nullptr, 0, nullptr, 0}
};
// clang-format on

const char* shortOptions = "i:p:v:f:s:d:r:m:t:q";

ConsoleArgs parseConsoleArguments(int argc, char* argv[]) {
    ConsoleArgs args;

    int c;
    while ((c = getopt_long(argc, argv, shortOptions, longOptions, NULL)) !=
           -1) {
        switch (c) {
            case 'i':
                args.inputFile = optarg;
                break;
            case 'p':
                args.pType = parseType(optarg);
                break;
            case 'v':
                args.velocityType = parseType(optarg);
                break;
            case 'f':
                args.velocityFlowType = parseType(optarg);
                break;
            case 'd':
                args.saveDir = optarg;
                break;
            case 'r':
                args.saveRate = std::stoul(optarg);
                break;
            case 's':
                args.saveFile = optarg;
                break;
            case 'm':
                args.maxIterations = std::stoul(optarg);
                break;
            case 'q':
                args.quiet = true;
                break;
            case 't':
                args.threads = std::stoul(optarg);
                break;
            case -1:
            default:
                throw invalid_argument("Invalid option");
        }
    }

    return args;
}

pair<bool, string> ConsoleArgs::validate() {
    if (!inputFile.empty() && !saveFile.empty()) {
        return {false,
                "Both --file and --use-save options cannot be provided."};
    }
    if (inputFile.empty() && saveFile.empty()) {
        return {false, "--file or --use-save option is required."};
    }
    if (saveRate == 0) {
        return {false, "--save-rate option must be greater than 0."};
    }
    if (threads == 0) {
        return {false, "--threads option must be greater than 0."};
    }

    return {true, ""};
}