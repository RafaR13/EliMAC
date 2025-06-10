# EliMAC

Rafael Ribeiro ist1102975

# TODO
- implement EliMAC
- implement precomputation
- test variances in the algorithm, such as precomputation, to see what changes in performance or security

```
sudo echo 0 > /sys/devices/system/cpu/cpufreq/boost
cat /sys/devices/system/cpu/cpufreq/boost

sudo cpupower frequency-set -g performance
cpupower frequency-info

taskset -c 0-7 ./elimac_csv --test --encoding 2 --parallel

perf stat -e cache-misses,L1-dcache-load-misses ./elimac_csv --test --encoding 2
```




# Usage
To compile and generate two executables (.txt output and .csv output)
```bash
make
```

To run the full test suite
```bash
./elimac_text --test [--parallel]
```

To run a specific message (with options)
```bash
./elimac_text --run [--message "My custom message"] [--random-keys] [--precompute] [--parallel] [--tag-bits 64]
```

To compile, run and output to .txt
```bash
make run_text
```

To compile, run and output to .csv
```bash
make run_csv
```

To compile, run and output to both files
```bash
make run_all
```

To delete executables and .o files
```bash
make clean
```
