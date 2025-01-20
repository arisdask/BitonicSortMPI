# BitonicSortMPI


## Introduction

### Problem Overview
The goal of this project is to implement a distributed sorting algorithm with bitonic interchanges. Given $p$ processes on the [Aristotelis HPC system](https://hpc.it.auth.gr/), each process is assigned an array of $q$ random numbers. The task is to sort all $p \times q$ elements using inter-process communication leveraging the Hamming distance (the data exchanges follow the edges of a hypercube, whose dimensionality corresponds to
the binary logarithm of the number of processes $p$).

Each process manages its data and coordinates with others to sort the dataset using [MPI](https://en.wikipedia.org/wiki/Message_Passing_Interface). By the end, data is sorted and distributed: process 0 holds the first segment, process 1 the next, and so on, ensuring contiguous distribution of the sorted dataset across processes.


### Visual Representation
Processes and their data can be visualized as a $p \times q$ matrix $A$:
- **Rows:** Processes  
- **Columns:** Numbers held by each process  

This abstraction helps to illustrate the inter-process communication and algorithm behavior. *More details about the algorithmic implementation can be found in the `BitonicSortMPI.pdf`*.


## Repository Layout
The repository contains the following:

- **MPI Bitonic Sort Implementation:**  
  Written in C, with a main function validating the sorted output. See `inc` and `src` folders.
  
- **Test Scripts:**  
  Scripts to build and run tests for various configurations (nodes, tasks per node, elements). See [Running the Scripts](#running-the-scripts) section.

- **Log Files and Results:**  
  Logs from Aristotelis HPC runs documenting runtime and correctness. A summary is in `logs/report.md`.

- **Julia Implementation:**  
  A Julia Pluto notebook demonstrates the first steps of implementing the algorithm. Find it in the `julia` folder.



## Cloning the Repository
This repository can be integrated with the Aristotelis HPC system in the following ways:  

1. **Direct Cloning on the HPC System:**  
   After connecting to the Aristotelis HPC system, clone the repository directly into your home directory using the command:
   ```bash
   git clone https://github.com/arisdask/BitonicSortMPI.git
   ```
2. **Local Cloning and Uploading:**  
   Alternatively, you can clone the repository to your local machine:  
   ```bash
   git clone https://github.com/arisdask/BitonicSortMPI.git
   ```  
   Then, upload the repository to the Aristotelis HPC system using a secure file transfer tool such as `scp` or `rsync`:  
   ```bash
   scp -r BitonicSortMPI [username]@aristotle.it.auth.gr:/path/to/home/
   ```
Replace `username` and `/path/to/home/` with your actual HPC credentials and desired directory.


## Running the Scripts

The repository provides several scripts to execute the Bitonic Sort implementation in C. Below are the available scripts and their functionalities:

### 1. **Run Bitonic Sort *Locally* with Specified Parameters**
This script allows you to specify the number of elements per process ($2^q$) and the number of processes ($2^p$) to run the Bitonic Sort. *It cleans, builds, and executes the program.*

**Usage:**
```bash
bash bash-run-local-bitonic-mpi.sh <q> <p>
```

- `<q>`: Number of elements per process $2^q$
- `<p>`: Number of processes $2^p$

**Example:**
```bash
bash bash-run-local-bitonic-mpi.sh 20 3
```

### 2. **Submit Test Cases**
This script submits the predefined test cases (which can be found in the `tests` folder) or a range of test cases using the HPC system's `sbatch` command. *It cleans, builds, and submits the jobs.*

**Usage:**
```bash
bash bash-submit-test-cases.sh <i> [j]
```

- `<i>`: Index of the test case to run. Use `-1` to run all test cases.
- `[j]`: (Optional) End index for a range of test cases. Defaults to the value of `<i>`.

**Examples:**
- Run all test cases:
  ```bash
  bash bash-submit-test-cases.sh -1
  ```
- Run a single test case (e.g., `testcase3`):
  ```bash
  bash bash-submit-test-cases.sh 3
  ```
- Run a range of test cases (e.g., `testcase2` to `testcase5`):
  ```bash
  bash bash-submit-test-cases.sh 2 5
  ```

### 3. **Submit a *custom* SLURM Job**
This script submits a single SLURM batch job to execute the Bitonic Sort implementation with specified custom HPC resources (e.g., nodes, tasks per node, CPUs per task). It includes options for configuring UCX transport settings to address potential connectivity issues (*more details about this kind of issues can be found in the next section*).

**Usage:**
Modify the script variables to set the number of processes, nodes, and other SLURM options. Then run:
```bash
sbatch sbatch_hpc_bitonic_mpi.sh
```
_**Remember** to `make clean` and `make` the project after any code implementation changes while using this script._

>**Keep in mind** when we change the number of processes $2^p$ we should also change the configurations ($\texttt{nodes} \times \texttt{ntasks-per-nodes} = 2^p$) in the SLURM script to align with our changes.


## Handling Potential Communication Issues

When testing the code, communication or connectivity issues might occur (*inter-node issues in most cases*), often due to high traffic on the HPC network. To mitigate these issues, you can add specific configurations to your job submission scripts. Below are the recommended ways to solve such problems:

1. **Set UCX Transport to TCP:**  
   Forces the communication to use TCP, which provides *more reliable transport*.  
   ```bash
   export UCX_TLS=tcp
   ```

2. **Increase UCX Rendezvous Timeout:**  
   Extends the timeout for establishing connections, which can help in congested networks.  
   ```bash
   export UCX_RNDV_TIMEOUT=15000  # Timeout in milliseconds
   ```

Include ***one of the above*** lines in the job submission script (e.g., `sbatch_hpc_bitonic_mpi.sh`) before the command that runs the program.

### Configuring UCX Transport for All Test Cases

To modify all test cases within the `/tests` directory to use TCP transport for inter-node communication, a script is provided. This script can add or remove the following line in each test case:

```bash
export UCX_TLS=tcp
```

#### Usage

1. **_Add_ the TCP Transport Line:**
   To enable TCP transport in all test cases, run the following command:
   ```bash
   bash add_tcp_transport.sh 1
   ```

2. **_Remove_ the TCP Transport Line:**
   To remove the TCP transport configuration from all test cases, run:
   ```bash
   bash add_tcp_transport.sh 0
   ```

#### How It Works
- **Option `1`:** Adds the `export UCX_TLS=tcp` line to all relevant test scripts in the `/tests` directory.  
- **Option `0`:** Removes the `export UCX_TLS=tcp` line from all test scripts in the `/tests` directory.  

This ensures that you can easily toggle the TCP transport setting across all test cases for troubleshooting or performance tuning.
