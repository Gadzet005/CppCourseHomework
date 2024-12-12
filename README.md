# Simple fluid simulation
## Example usage:
- Build
```
cmake -S . -B build -DTYPES="FIXED(64, 32), DOUBLE, FAST_FIXED(50, 5)" -DSIZES="S(36,84), S(1980, 1000)"
```
- Start simulation with config text file (input.example.txt)
```
./main --file "./env/input.txt" --p-type="fixed(64, 32)" --v-type="fixed(64, 32)" --v-flow-type="fixed(64, 32)"
```
- Save simulation state to binary file
```
./main --file "./env/input.txt" --save-dir="./save" --save-rate=100
```
- Load simulation state from binary file
```
./main --use-save="./save/1"
```