#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <mpi.h>
#include "../inc/utils.h"
#include "../inc/row_sort_operations.h"
#include "../inc/bitonic_sort.h"
#include "../inc/validation.h"


int main(int argc, char* argv[]) {
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Ensure all processes are ready before starting
    MPI_Barrier(MPI_COMM_WORLD);

    // Use rank and hostname to generate a unique seed for each process
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    srand(time(NULL) + rank + strlen(hostname));

    if (argc != 3) {
        if (rank == 0) printf("Usage: %s <q: 2^q numbers/process> <p: 2^p processes>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    int total_cols = 1 << atoi(argv[1]);  // Colums are the number of elements each process has
    int total_rows = 1 << atoi(argv[2]);  // Rows are the total number of processes

    int* local_row = malloc(total_cols * sizeof(int));
    for (int i = 0; i < total_cols; i++) {
        local_row[i] = rand() % RAND_MAX;
    }

    // // Ensure all processes have initialized their data
    // MPI_Barrier(MPI_COMM_WORLD);
    // print_row(local_row, total_cols, rank, size);


    MPI_Barrier(MPI_COMM_WORLD);
    double startTime = MPI_Wtime();

    bitonic_sort(local_row, total_rows, total_cols, rank);
    
    double localEndTime = MPI_Wtime();
    double localTime = localEndTime - startTime;
    MPI_Barrier(MPI_COMM_WORLD);

    // Find the maximum time across all processes
    double maxTime;
    MPI_Reduce(&localTime, &maxTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Sorting Time: %f msec\n", maxTime * 1000);
        fflush(stdout);
    }

    // MPI_Barrier(MPI_COMM_WORLD);
    // print_row(local_row, total_cols, rank, size);

    MPI_Barrier(MPI_COMM_WORLD);

    bool eval_flag = true;
    validate_bitonic_sort(local_row, total_cols, rank, size, &eval_flag);

    // Variable to store the reduced result at rank 0
    bool global_eval_flag = true;

    // Perform logical AND reduction of all eval_flags to rank 0
    MPI_Reduce(&eval_flag, &global_eval_flag, 1, MPI_C_BOOL, MPI_LAND, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        if (global_eval_flag) {
            printf("\nSorting: Correct!!!\n");
        } else {
            printf("\nSorting: Incorrect :(\n");
        }
    }

    free(local_row);

    MPI_Finalize();
    return 0;
}