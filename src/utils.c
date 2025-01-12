#include <../inc/utils.h>


// Local sort for a single row
void local_sort(int* row, int cols, bool ascending) {
    for (int i = 0; i < cols - 1; i++) {
        for (int j = 0; j < cols - i - 1; j++) {
            if (ascending) {
                if (row[j] > row[j + 1]) {
                    int temp = row[j];
                    row[j] = row[j + 1];
                    row[j + 1] = temp;
                }
            } else {
                if (row[j] < row[j + 1]) {
                    int temp = row[j];
                    row[j] = row[j + 1];
                    row[j + 1] = temp;
                }
            }
        }
    }
}


// Function to perform the initial alternating sort on each local row
void initial_alternating_sort(int* row, int cols, int rank) {
    bool ascending = (rank % 2 == 0);
    local_sort(row, cols, ascending);
}


void print_row(int* row, int cols, int rank, int size) {
    MPI_Barrier(MPI_COMM_WORLD);

    for (int i = 0; i < size; i++) {
        if (rank == i) {
            // Only the current rank prints its row
            size_t buffer_size = 1000;
            char* buffer = (char*)malloc(buffer_size);
            if (!buffer) {
                fprintf(stderr, "Rank %d: Memory allocation failed\n", rank);
                return;
            }

            size_t offset = 0;
            offset += snprintf(buffer + offset, buffer_size - offset, "Rank %d: ", rank);

            for (int j = 0; j < cols; j++) {
                offset += snprintf(buffer + offset, buffer_size - offset, "%d, ", row[j]);
                if (offset >= buffer_size - 1) {
                    fprintf(stderr, "Rank %d: Buffer size exceeded\n", rank);
                    free(buffer);
                    return;
                }
            }

            snprintf(buffer + offset, buffer_size - offset, "\n");
            printf("%s", buffer);
            fflush(stdout); // Immediate flush to avoid interleaving

            free(buffer);
        }
        // Synchronize between ranks
        MPI_Barrier(MPI_COMM_WORLD);
    }
}


