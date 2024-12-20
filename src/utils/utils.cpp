#include "utils.hpp"

#include <fstream>
#include <simulation/save_load.hpp>

using namespace std;

FluidSimulationState loadStateByArgs(const ConsoleArgs& args) {
    FluidSimulationState state;

    if (!args.inputFile.empty()) {
        // Load initial state of simulation from text file.
        ifstream in;
        in.open(args.inputFile);
        if (!in.is_open()) {
            throw runtime_error("Error opening file" + args.inputFile);
        }
        state = loadFluidSimulationStartState(in);
        cout << "Successfully loaded start state of simulation." << endl;
    } else {
        // Load saved state of simulation from bin file.
        ifstream in;
        in.open(args.saveFile, ios::binary);
        if (!in.is_open()) {
            throw runtime_error("Error opening file" + args.saveFile);
        }
        state = loadFluidSimulationState(in);
        cout << "Successfully loaded state of simulation, tickCount = "
             << state.tickCount << "." << endl;
    }

    return state;
}

void saveStateByArgs(const ConsoleArgs& args,
                     const FluidSimulationState& state) {
    string saveFilePath = args.saveDir + "/" + to_string(state.tickCount);
    ofstream out;
    out.open(saveFilePath, ios::binary);
    if (!out.is_open()) {
        throw runtime_error("Save error occurred.");
    }
    saveFluidSimulationState(out, state);
    cout << "Saved state to file: " << saveFilePath << "." << endl;
}