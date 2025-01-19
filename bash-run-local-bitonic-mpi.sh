#!/bin/bash

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <q: 2^q numbers/process> <p: 2^p processes>"
    exit 1
fi

VALUE1=$1
VALUE2=$2

if ! [[ "$VALUE1" =~ ^[0-9]+$ ]] || ! [[ "$VALUE2" =~ ^[0-9]+$ ]]; then
    echo "Error: Both arguments must be positive integers."
    exit 1
fi

# Calculate 2^VALUE2
NUM_PROCESSES=$((2**VALUE2))

# Clean, build, and run the program:
make clean
module load gcc openmpi
make

if [ $? -ne 0 ]; then
    echo "Error: Build failed."
    exit 1
fi

# Run the program using mpirun
# Usage: $-np <2^p: 2^p processes> ./bin/bitonic_mpi <q: 2^q numbers/process> <p: 2^p processes>
mpirun -np "$NUM_PROCESSES" ./bin/bitonic_mpi "$VALUE1" "$VALUE2"
