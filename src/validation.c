#include "../inc/validation.h"

// Function to check if an array is sorted in ascending or descending order
bool is_localy_sorted(int* row, int cols, bool* is_ascending) {
    bool ascending = true;
    bool descending = true;

    for (int i = 1; i < cols; i++) {
        if (row[i] < row[i - 1]) {
            ascending = false; // Not ascending
        }
        if (row[i] > row[i - 1]) {
            descending = false; // Not descending
        }
        if (!ascending && !descending) {
            // If it's neither ascending nor descending, no need to continue
            break;
        }
    }

    if (ascending || descending) {
        *is_ascending = ascending; // Set whether the order is ascending
        return true;
    }
    return false; // Not sorted
}


// Validates the correctness of the distributed bitonic sort
void validate_bitonic_sort(int* local_row, int cols, int rank, int size, bool* eval) {
    *eval = true;

    // Step 1: Check if the local row is sorted in ascending order
    for (int i = 0; i < cols - 1; i++) {
        if (local_row[i] > local_row[i + 1]) {
            fprintf(stderr, "Validation failed: Rank %d's local row is not sorted.\n", rank);
            fflush(stderr);
            *eval = false;
            break;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    // Step 2: Check global order between ranks
    int local_last_element = local_row[cols - 1];
    int previous_last_element = 0;

    if (rank < size - 1) {
        // Send the last element to the next rank
        MPI_Send(&local_last_element, 1, MPI_INT, rank + 1, 0, MPI_COMM_WORLD);
    }

    if (rank > 0) {
        // Receive the last element from the previous rank
        MPI_Recv(&previous_last_element, 1, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Validate global order with the previous rank
        if (previous_last_element > local_row[0]) {
            fprintf(stderr, "Validation failed: Rank %d's first element (%d) is smaller than Rank %d's last element (%d).\n",
                   rank, local_row[0], rank - 1, previous_last_element);
            fflush(stderr);
            *eval = false;
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
}
