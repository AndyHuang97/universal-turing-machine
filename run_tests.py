import subprocess
from os import listdir
from os.path import isfile, join
import re

# Compile the C program
# Define the EVAL constant to remove final sorting
program = "src/main"
add_eval = False
eval = "-DEVAL" if add_eval else ""

compilation = f"gcc -Wall -Werror -O2 -g3 {eval} {program}.c -o {program}"
subprocess.call(compilation, shell=True)

path = "./tests"
input_path = path + "/input"
output_path = path + "/output"
files = [f for f in listdir(input_path) if isfile(join(input_path, f))]

# Get the input files
r = re.compile('input_.*')
input_files = list(filter(lambda f: r.match(f), files))
input_files.sort()
# print(input_files)

# Run the tests, which expects the output to be sorted in increasing order
for file in input_files:
    print(f">>> Testing {file}")
    f = open(f"{path}/input/{file}")

    p = subprocess.run([f"./{program}"],
                            input=f.read(),
                            text=True,
                            capture_output=True, 
                            )
    f.close()
    file_idx = file.split("_")[1]
    program_output = f"{output_path}/computed_output_{file_idx}"
    write_f = open(program_output, "w")
    write_f.write(p.stdout)
    write_f.close()
    #print(p.stdout)
    print("The exit code was: %d\n" % p.returncode)

    true_output = f"{output_path}/output_{file_idx}"
    diff = subprocess.run(["diff", true_output, program_output])