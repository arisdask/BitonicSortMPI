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

# Clean, build, and run the program:
make clean
make

if [ $? -ne 0 ]; then
    echo "Error: Build failed."
    exit 1
fi

mpirun -np 16 ./bin/bitonic_mpi "$VALUE1" "$VALUE2"
