# Final project for the API 2017-2018
This is the final project for the course on "Algorithms and Data structures".
The task is to develop a non-determinstic Universal Turing Machine (UTM).
## Task description
### Input
The input provides:
- a description of a machine is provided for each test i.e. the state transition function.
- the termination states.
- the max depth of a computation, after which the UTM can assume the presence of a loop and terminate with an undefined ("U") output.
- a list of input strings to feed to the given machine

### Output
Given the input, the UTM must simulate the described machine and print the output strings for each the input string.



## How to run
Open command line:
```
cd to/this/folder
bash compile_and_run.sh
```

## Run tests
```
python run_tests.py
```