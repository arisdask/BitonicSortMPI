#!/bin/bash
#SBATCH --partition=rome
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=32
#SBATCH --cpus-per-task=1
#SBATCH --time=00:15:00
#SBATCH --output=logs/case_27_5_%j.out
#SBATCH --error=logs/case_27_5_%j.err

# Load required modules
module load gcc openmpi

# Set environment variables
export UCX_WARN_UNUSED_ENV_VARS=n

# In case of `UCX  ERROR connect(fd=.., dest_addr=..) failed: Connection timed out` 
# uncomment *one* of the following exports (recommended to use tcp) to ensure "safer" transportation of data:
# export UCX_RNDV_TIMEOUT=5000     # Time in milliseconds

# Run the program using srun
# Usage: $0 <q: 2^q numbers/process> <p: 2^p processes>
# When we change the number of processes (2^p) we should set the nodes/ntasks-per-node
srun ./bin/bitonic_mpi 27 5
