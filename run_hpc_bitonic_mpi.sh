#!/bin/bash
#SBATCH --partition=batch
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=4
#SBATCH --cpus-per-task=1
#SBATCH --time=00:00:10
#SBATCH --output=logs/bitonic_sort_%j.out
#SBATCH --error=logs/bitonic_sort_%j.err

# Load required modules
module load gcc openmpi

# Set environment variables
export UCX_WARN_UNUSED_ENV_VARS=n

# Clean, build, and check for errors
make clean
make

if [ $? -ne 0 ]; then
    echo "Error: Build failed."
    exit 1
fi

# Run the program using srun
# Usage: $0 <q: 2^q numbers/process> <p: 2^p processes>
# When we change the number of processes (2^p) we should set the nodes/ntasks-per-node
srun ./bin/bitonic_mpi 4 3
