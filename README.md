# EliMAC

Rafael Ribeiro ist1102975

## Important notes
- It is better to run the program to csv output, since it's clearer to read, and allows for the usage of the python script to generate graphs.
- I left a .csv file inside out/, that contains the results of running a full test suite (tests all configurations)
- You should change the frequency value in both src/main.h and analyze_csv.py to your CPU's frequency
- make clean removes everything (executables, .o's, out/, tables/ and graphs/, so if you want to save any of them, copy them somewhere else so you don't have to run everything again).
- (Teacher specific information) Here is the link to the repository: https://github.com/RafaR13/EliMAC just in case.

## To compile and run the full test suite
```bash
make clean && make run
```

## Compilation
### Default (no OpenMP)
```bash
make clean && make
```
### Using OpenMP (for --parallel)
```bash
make clean && make PARALLEL=1
```
Output: ```./elimac```

## Run
### Test Suite (CSV output)
```bash
make run
```
Output: ```out/elimac_results.csv```
### Test Suite (Text output)
```bash
make run_txt
```
Output: ```out/elimac_results.txt```

<b>Note: AVOID USING THE TEXT OUTPUT FORMAT AS IT'S STILL NOT COMPLETELY CORRECT</b>
### Single Message
```bash
taskset -c 0-7 ./elimac --run --message "Test" --output-format txt
```
#### Options
- ```--random-keys```: Use random keys
- ```--precompute```: Enable precomputation
- ```--parallel```: Enable parallel processing
- ```--tag-bits <32|64|96|128>```: Tag size
- ```--encoding <0|1|2|3|4>```: Naive (0), Compact (1), Custom1 (2), Custom2 (3), All (4)
- ```--output-format <txt|csv>```: Output format

Output: ```out/elimac_results.csv```

## Profilling
### To check ```CyclesPerByte```
```bash
perf stat -e cycles,instructions ./elimac --test --encoding 2 --parallel --output-format csv
```
### For detailed analysis
```bash
perf record ./elimac --test --encoding 4 --parallel --output-format csv
perf report
```

## Notes
- Results are saved in out/.
- Use taskset for consistent CPU affinity.
- Analyze CSV with python3 analyze_csv.py to get graphs.
