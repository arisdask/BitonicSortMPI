#include <../inc/bitonic_sort.h>


// New Version:
void bitonic_sort(int* local_row, int rows, int cols, int rank) {
    int stages = (int)log2(rows);
    
    // Pre-allocate buffer for communications
    int* received_row = malloc(cols * sizeof(int));
    if (!received_row) {
        fprintf(stderr, "Rank %d: Memory allocation failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    // Step 1: Initial alternating sorting
    initial_alternating_sort(local_row, cols, rank);
    MPI_Barrier(MPI_COMM_WORLD);

    // Step 2: Iterative bitonic stages
    for (int stage = 1; stage <= stages; stage++) {
        int num_chunks = 1 << (stages - stage);
        int chunk_size = rows / num_chunks;
        int chunk = rank / chunk_size;
        bool is_ascending = (chunk % 2 == 0);

        // Communication based on Hamming distance for recursive steps
        for (int step = stage - 1; step >= 0; step--) {
            int partner = rank ^ (1 << step);  // Compute partner based on Hamming distance
            
            if (partner < rows) {  // Ensure partner rank is valid
                // Use MPI_Sendrecv for deadlock-free communication
                MPI_Status status;
                int result = MPI_Sendrecv(
                    local_row, cols, MPI_INT, partner, 0,
                    received_row, cols, MPI_INT, partner, 0,
                    MPI_COMM_WORLD, &status
                );

                if (result != MPI_SUCCESS) {
                    fprintf(stderr, "Rank %d: MPI_Sendrecv failed\n", rank);
                    fflush(stderr);
                    MPI_Abort(MPI_COMM_WORLD, -1);
                }

                // Determine sorting direction based on rank comparison
                bool sort_ascending = (rank < partner) ? is_ascending : !is_ascending;
                pairwise_sort(local_row, received_row, cols, sort_ascending);
            }

            MPI_Barrier(MPI_COMM_WORLD);
        }

        elbow_sort(local_row, cols, is_ascending);
        MPI_Barrier(MPI_COMM_WORLD);
    }

    // Clean up
    free(received_row);
}


// void bitonic_sort(int* local_row, int rows, int cols, int rank) {
//     int stages = (int)log2(rows);

//     // Step 1: Initial alternating sorting
//     initial_alternating_sort(local_row, cols, rank);
//     MPI_Barrier(MPI_COMM_WORLD);

//     // Step 2: Iterative bitonic stage
//     for (int stage = 1; stage <= stages; stage++) {
//         int numOfChunks = 1 << (stages - stage);
//         int sizeOfChunk = rows / numOfChunks;

//         // Determine if this chunk is sorted in ascending or descending order
//         int chunk = rank / sizeOfChunk;
//         bool isAscending = (chunk % 2 == 0);
//         int recursiveStep = 1 << (stage - 1);

//         MPI_Barrier(MPI_COMM_WORLD);

//         while (recursiveStep >= 1) {
//             int partner_rank = rank ^ recursiveStep;

//             if (rank < partner_rank) {
//                 // Rank i sends its row to rank i + recursiveStep
//                 MPI_Send(local_row, cols, MPI_INT, partner_rank, 0, MPI_COMM_WORLD);

//                 // Rank i receives the sorted row back from rank i + recursiveStep
//                 MPI_Recv(local_row, cols, MPI_INT, partner_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//             } else {
//                 // Rank i + recursiveStep receives the row from rank i
//                 int* received_row = malloc(cols * sizeof(int));
//                 MPI_Recv(received_row, cols, MPI_INT, partner_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

//                 // Perform pairwise sort
//                 pairwise_sort(local_row, received_row, cols, isAscending);

//                 // Rank i + recursiveStep sends the sorted portion back to rank i
//                 MPI_Send(received_row, cols, MPI_INT, partner_rank, 0, MPI_COMM_WORLD);

//                 free(received_row);
//             }

//             // Move to the next level of recursion
//             recursiveStep >>= 1;

//             // Wait for all processes to synchronize before the next step
//             MPI_Barrier(MPI_COMM_WORLD);
//         }

//         // Elbow sort at the end of each stage to finalize local ordering
//         elbow_sort(local_row, cols, isAscending);
//         MPI_Barrier(MPI_COMM_WORLD);
//     }

//     MPI_Barrier(MPI_COMM_WORLD);
// }



// // Never tested version 0.1:
// void bitonic_sort(int* local_row, int rows, int cols, int rank) {
//     int stages = (int)log2(rows);

//     // Step 1: Initial alternating sorting
//     initial_alternating_sort(local_row, cols, rank);
//     MPI_Barrier(MPI_COMM_WORLD);

//     // Step 2: Iterative bitonic stages
//     for (int stage = 1; stage <= stages; stage++) {
//         int num_chunks = 1 << (stages - stage);
//         int chunk_size = rows / num_chunks;
//         int chunk = rank / chunk_size;

//         bool is_ascending = (chunk % 2 == 0);

//         // Communication based on Hamming distance for recursive steps
//         for (int step = stage - 1; step >= 0; step--) {
//             int partner = rank ^ (1 << step); // Compute partner based on Hamming distance

//             if (partner < rows) { // Ensure partner rank is valid
//                 int* received_row = malloc(cols * sizeof(int));
//                 if (!received_row) {
//                     fprintf(stderr, "Rank %d: Memory allocation failed\n", rank);
//                     MPI_Abort(MPI_COMM_WORLD, -1);
//                 }

//                 // Determine sender/receiver based on rank comparison
//                 if (rank < partner) {
//                     MPI_Sendrecv(local_row, cols, MPI_INT, partner, 0, 
//                                  received_row, cols, MPI_INT, partner, 0, 
//                                  MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//                     pairwise_sort(local_row, received_row, cols, is_ascending);
//                 } else {
//                     MPI_Sendrecv(local_row, cols, MPI_INT, partner, 0, 
//                                  received_row, cols, MPI_INT, partner, 0, 
//                                  MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//                     pairwise_sort(local_row, received_row, cols, !is_ascending);
//                 }

//                 free(received_row);
//             }

//             // Ensure all processes synchronize at each step
//             MPI_Barrier(MPI_COMM_WORLD);
//         }

//         // Elbow sort at the end of each stage to finalize local ordering
//         elbow_sort(local_row, cols, is_ascending);
//         MPI_Barrier(MPI_COMM_WORLD);
//     }
// }




// // Incomplete version 0:
// void bitonic_sort(int* local_row, int rows, int cols, int rank) {
//     int stages = (int)log2(rows);
    
//     // Step 1: Initial alternating sorting
//     initial_alternating_sort(local_row, cols, rank);
//     MPI_Barrier(MPI_COMM_WORLD);
    
//     // Step 2: Iterative bitonic stage
//     for (int stage = 1; stage <= stages; stage++) {
//         int numOfChunks = 1 << (stages - stage);
//         int sizeOfChunk = rows / numOfChunks;

//         // Each rank/row belongs to a specific chunk in each stage
//         // which can run in parallel with the others chunks
//         int chunk = rank / sizeOfChunk;

//         bool isAscending = (chunk % 2 == 0);
//         int recursiveStep = 1 << (stage - 1);
//         int counter = 0;

//         MPI_Barrier(MPI_COMM_WORLD);
        
//         while (recursiveStep >= 1) {
//             int numOfCopies = 1 << counter;
            
//             int k = rank % ( (sizeOfChunk >> 1) >> counter );
            
//             int i = k + chunk * sizeOfChunk;

//             if (rank - chunk * sizeOfChunk == k) {
//                 int* received_row = malloc(cols * sizeof(int));

//                 MPI_Recv(received_row, cols, MPI_INT, rank + recursiveStep, 0, 
//                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
//                 pairwise_sort(local_row, received_row, cols, isAscending);
//                 MPI_Send(received_row, cols, MPI_INT, rank + recursiveStep, 0, 
//                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

//             }

//             recursiveStep = recursiveStep >> 1;
//             counter++;

//             // At each recursive step we should wait for all processes to finish before moving on to the next one
//             MPI_Barrier(MPI_COMM_WORLD);
//         }
        
//         // Elbow sort at each iteration
//         for (int i = 0; i < localRows; i++) {
//             int globalRow = rank * localRows + i;
//             int pattern_length = 1 << (stage + 1);
//             int pattern_index = globalRow % pattern_length;
//             bool ascending = pattern_index < (pattern_length / 2);
//             elbow_sort(localData[i], cols, !ascending);
//         }
        
//         MPI_Barrier(MPI_COMM_WORLD);
//     }
// }
