# EliMAC

Rafael Ribeiro ist1102975

# TODO
- implement EliMAC
- implement precomputation
- test variances in the algorithm, such as precomputation, to see what changes in performance or security

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
