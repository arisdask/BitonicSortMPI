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
        local_row[i] = rand() % 300;  // % RAND_MAX
    }

    // Ensure all processes have initialized their data
    // MPI_Barrier(MPI_COMM_WORLD);
    // print_row(local_row, total_cols, rank, size);


    MPI_Barrier(MPI_COMM_WORLD);
    double startTime = MPI_Wtime();

    bitonic_sort(local_row, total_rows, total_cols, rank);

    // Ensure all processes have finished sorting
    MPI_Barrier(MPI_COMM_WORLD);
    double endTime = MPI_Wtime();


    if (rank == 0) {
        printf("\nSorting All Rows: Done.\n");
        printf("Sorting time: %f seconds\n", endTime - startTime);
    }
    
    // Ensure header is printed
    // MPI_Barrier(MPI_COMM_WORLD);
    // print_row(local_row, total_cols, rank, size);
    MPI_Barrier(MPI_COMM_WORLD);

    bool is_ascending;

    if (is_localy_sorted(local_row, total_cols, &is_ascending)) {
        printf("-> Rank: %d is sorted in order: %s.\n", rank, is_ascending ? "Ascending" : "Descending!!");
    } else {
        printf("-> Rank: %d is not sorted.\n", rank);
    }
    fflush(stdout);

    MPI_Barrier(MPI_COMM_WORLD);
    validate_bitonic_sort(local_row, total_cols, rank, size);    
    MPI_Barrier(MPI_COMM_WORLD);

    free(local_row);

    MPI_Finalize();
    return 0;
}