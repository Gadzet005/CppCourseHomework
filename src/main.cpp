#include <cli/console_args_parser.hpp>
#include <iostream>
#include <simulation/loader.hpp>
#include <types/fast_fixed.hpp>

using namespace std;

constexpr size_t N = 36, M = 84;

int main(int argc, char* argv[]) {
    auto args = parseConsoleArguments(argc, argv);

    if (args.filePath.empty()) {
        cout << "Error: Input file path is not provided" << endl;
        return 0;
    }

    auto description = loadSimulationDescription(args.filePath);
    FluidSimulation<N, M, FastFixed<64, 32>, FastFixed<64, 32>,
                    FastFixed<64, 32>>
        simulation(description);

    for (size_t i = 0;; ++i) {
        if (simulation.step()) {
            cout << "Tick " << i << ":\n";
            simulation.print_field();
        }
    }

    return 0;
}