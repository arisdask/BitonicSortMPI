#include "../inc/utils.h"


// Local sort for a single row
void local_sort(int* row, int cols, bool ascending) {
    #if SORT_VERSION == 0
        if (row == NULL || cols <= 0) {
            return;
        }
        
        // Comparison functions definitions
        int compare_asc(const void* a, const void* b)  { return (*(int*)a - *(int*)b); }
        int compare_desc(const void* a, const void* b) { return (*(int*)b - *(int*)a); }
        
        // Choose comparison function based on sort direction
        int (*compare_func)(const void*, const void*) = ascending ? compare_asc : compare_desc;
        
        // Call qsort with appropriate comparison function
        qsort(row, cols, sizeof(int), compare_func);

    #else
        // TODO: multithreaded sorting
    #endif
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
            size_t buffer_size = 1024;
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
