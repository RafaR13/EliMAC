# EliMAC

Rafael Ribeiro ist1102975

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
- ```--encoding <0|1|2>```: Naive (0), Compact (1), Both (2)
- ```--output-format <txt|csv>```: Output format

Output: ```out/elimac_results.csv```

## Profilling
### To check ```CyclesPerByte```
```bash
perf stat -e cycles,instructions ./elimac --test --encoding 2 --parallel --output-format csv
```
### For detailed analysis
```bash
perf record ./elimac --test --encoding 2 --parallel --output-format csv
perf report
```

## Notes
- Results are saved in out/.
- Use taskset for consistent CPU affinity.
- ~~Expected CyclesPerByte: ~0.5–2 (target: 0.13–0.46).~~
- Analyze CSV with python3 analyze_csv.py to get graphs.
