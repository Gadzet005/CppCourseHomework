#include <cli/console_args_parser.hpp>
#include <iostream>
#include <simulation/factory.hpp>
#include <simulation/loader.hpp>
#include <types/fast_fixed.hpp>

using namespace std;

int main(int argc, char* argv[]) {
    auto args = parseConsoleArguments(argc, argv);

    if (args.filePath.empty()) {
        cout << "Error: Input file path is not provided" << endl;
        return 0;
    }

    SimulationState state = loadSimulationState(args.filePath);
    FactoryContext ctx = {state.getHeight(), state.getWidth(),      args.pType,
                          args.velocityType, args.velocityFlowType, state};
    auto simulation = FluidSimulationFactory(ctx).create();

    cout << "Press anything to start." << endl;
    cout << "Press Ctrl+C to stop." << endl;
    getchar();

    simulation->print_field();
    for (size_t i = 1;; ++i) {
        if (simulation->step()) {
            cout << "Tick " << i << ":\n";
            simulation->print_field();
        }
    }

    return 0;
}