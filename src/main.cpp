#include <cli/console_args.hpp>
#include <iostream>
#include <simulation/factory.hpp>

#include "utils/utils.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);

    auto args = parseConsoleArguments(argc, argv);
    auto [valid, error] = args.validate();
    if (!valid) {
        cout << error << endl;
        return 0;
    }

    FluidSimulationState state = loadStateByArgs(args);
    FactoryContext ctx = {state.getFieldHeight(),
                          state.getFieldWidth(),
                          args.pType,
                          args.velocityType,
                          args.velocityFlowType,
                          args.threads,
                          state};
    auto simulation = FluidSimulationFactory(ctx).create();

    cout << "\nPress anything to start." << endl;
    cout << "Press Ctrl+C to stop." << endl;
    getchar();

    cout << "Tick " << simulation->getTickCount() << endl;
    if (!args.quiet) {
        simulation->print_field();
    }

    while (simulation->getTickCount() < args.maxIterations) {
        if (!simulation->step()) {
            continue;
        }

        cout << "Tick " << simulation->getTickCount() << endl;
        if (!args.quiet) {
            simulation->print_field();
        }

        if (simulation->getTickCount() != 0 &&
            simulation->getTickCount() % args.saveRate == 0) {
            // Save state of simulation to bin file.
            saveStateByArgs(args, simulation->getState());
        }
    }

    return 0;
}