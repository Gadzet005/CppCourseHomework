# Simple fluid simulation
## Example usage:
- Build
```
cmake -S . -B build -DTYPES="FIXED(64, 32), DOUBLE, FAST_FIXED(50, 5)" -DSIZES="S(36,84), S(1980, 1000)"
```
- Start simulation with config text file (input.example.txt)
```
./main -i "./env/input.txt" -p "FIXED(64, 32)" -v "FAST_FIXED(50, 5)" -f "DOUBLE"
```
- Save simulation state to binary file
```
./main -i "./env/input.txt" -d "./save" -r 100
```
- Load simulation state from binary file
```
./main -s "./save/1"
```
- Configure number of threads (default: 1)
```
./main -i "./env/input.txt" -t 10
```