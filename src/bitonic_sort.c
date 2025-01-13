#include <../inc/bitonic_sort.h>

#define CHUNK_SIZE 33554432

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

            if (rank != partner && partner < rows) {
                // Generate a unique tag for this stage and step
                int base_tag = (stage << 8) | step; // Combine stage and step into a unique tag
                MPI_Request send_request, recv_request;

                for (int offset = 0; offset < cols; offset += CHUNK_SIZE) {
                    int current_chunk_size = (offset + CHUNK_SIZE <= cols) ? CHUNK_SIZE : cols - offset;
                    int send_tag = base_tag + offset; // Adjust tag by offset for chunk
                    int recv_tag = base_tag + offset;

                    MPI_Isend(local_row + offset, current_chunk_size, MPI_INT, partner, send_tag, MPI_COMM_WORLD, &send_request);
                    MPI_Irecv(received_row + offset, current_chunk_size, MPI_INT, partner, recv_tag, MPI_COMM_WORLD, &recv_request);

                    MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
                    pairwise_sort(local_row + offset, received_row + offset, current_chunk_size, (rank < partner) ? is_ascending : !is_ascending);

                    MPI_Wait(&send_request, MPI_STATUS_IGNORE);
                }
            }

            // // Uncomment if needed for debugging synchronization issues
            // MPI_Barrier(MPI_COMM_WORLD);
        }

        elbow_sort(local_row, cols, is_ascending);
        // // Uncomment if needed for debugging synchronization issues
        // MPI_Barrier(MPI_COMM_WORLD);
    }

    // Clean up
    free(received_row);
}


// // Version 2 (tested localy - faster communcations with chunks)
// void bitonic_sort(int* local_row, int rows, int cols, int rank) {
//     int stages = (int)log2(rows);

//     // Pre-allocate buffer for communications
//     int* received_row = malloc(cols * sizeof(int));
//     if (!received_row) {
//         fprintf(stderr, "Rank %d: Memory allocation failed\n", rank);
//         MPI_Abort(MPI_COMM_WORLD, -1);
//     }

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
//             int partner = rank ^ (1 << step);  // Compute partner based on Hamming distance

//             if (rank != partner && partner < rows) {
//                 // Use non-blocking communication for pipelined data exchange
//                 MPI_Request send_request, recv_request;
//                 for (int offset = 0; offset < cols; offset += CHUNK_SIZE) {
//                     int current_chunk_size = (offset + CHUNK_SIZE <= cols) ? CHUNK_SIZE : cols - offset;

//                     MPI_Isend(local_row + offset, current_chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD, &send_request);
//                     MPI_Irecv(received_row + offset, current_chunk_size, MPI_INT, partner, 0, MPI_COMM_WORLD, &recv_request);

//                     MPI_Wait(&recv_request, MPI_STATUS_IGNORE);
//                     pairwise_sort(local_row + offset, received_row + offset, current_chunk_size, (rank < partner) ? is_ascending : !is_ascending);

//                     MPI_Wait(&send_request, MPI_STATUS_IGNORE);
//                 }
//             }

//             // MPI_Barrier(MPI_COMM_WORLD);
//         }

//         elbow_sort(local_row, cols, is_ascending);
//         // MPI_Barrier(MPI_COMM_WORLD);
//     }

//     // Clean up
//     free(received_row);
// }




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Version 1 (tested - works !!)
// void bitonic_sort(int* local_row, int rows, int cols, int rank) {
//     int stages = (int)log2(rows);
    
//     // Pre-allocate buffer for communications
//     int* received_row = malloc(cols * sizeof(int));
//     if (!received_row) {
//         fprintf(stderr, "Rank %d: Memory allocation failed\n", rank);
//         MPI_Abort(MPI_COMM_WORLD, -1);
//     }

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
//             int partner = rank ^ (1 << step);  // Compute partner based on Hamming distance
            
//             if (rank >= partner) {
//                 int result = MPI_Send(local_row, cols, MPI_INT, partner, 0, MPI_COMM_WORLD);
//                 if (result != MPI_SUCCESS) {
//                     fprintf(stderr, "Rank %d: MPI_Send failed\n", rank);
//                     fflush(stderr);
//                     MPI_Abort(MPI_COMM_WORLD, -1);
//                 }

//                 MPI_Status status;
//                 result = MPI_Recv(local_row, cols, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);
//                 if (result != MPI_SUCCESS) {
//                     fprintf(stderr, "Rank %d: MPI_Recv failed\n", rank);
//                     fflush(stderr);
//                     MPI_Abort(MPI_COMM_WORLD, -1);
//                 }
                
//             } else
//             if (rank < partner && partner < rows) {
//                 MPI_Status status;
//                 int result = MPI_Recv(received_row, cols, MPI_INT, partner, 0, MPI_COMM_WORLD, &status);
//                 if (result != MPI_SUCCESS) {
//                     fprintf(stderr, "Rank %d: MPI_Recv failed\n", rank);
//                     fflush(stderr);
//                     MPI_Abort(MPI_COMM_WORLD, -1);
//                 }

//                 pairwise_sort(local_row, received_row, cols, is_ascending);

//                 result = MPI_Send(received_row, cols, MPI_INT, partner, 0, MPI_COMM_WORLD);
//                 if (result != MPI_SUCCESS) {
//                     fprintf(stderr, "Rank %d: MPI_Send failed\n", rank);
//                     fflush(stderr);
//                     MPI_Abort(MPI_COMM_WORLD, -1);
//                 }
//             }

//             MPI_Barrier(MPI_COMM_WORLD);
//         }

//         elbow_sort(local_row, cols, is_ascending);
//         MPI_Barrier(MPI_COMM_WORLD);
//     }

//     // Clean up
//     free(received_row);
// }
