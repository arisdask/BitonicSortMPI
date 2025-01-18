#include "../inc/bitonic_sort.h"

// Version 4 (Optimized Communications and time measurements - tested)
#define CHUNK_DIVISOR        8      // Number of chunks to split the data into to transmit
#define PRINT_TIME_LOGS      0      // 0: Do not print | 1: prints time measurements' logs if they cost more than `MIN_TIME_THRESHOLD`
#define MIN_TIME_THRESHOLD   2.0    // Defines the minimum time threshold to be printed

void bitonic_sort(int* local_row, int rows, int cols, int rank) {
    int stages = (int)log2(rows);

    // Pre-allocate buffer for communications
    int* received_row = malloc(cols * sizeof(int));
    int* send_buff = malloc(cols * sizeof(int));
    if (!received_row) {
        fprintf(stderr, "Rank %d: Memory allocation failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    // Step 1: Initial alternating sorting
    double start_time = MPI_Wtime();
    initial_alternating_sort(local_row, cols, rank);
    double end_time = MPI_Wtime();
    if (end_time - start_time > MIN_TIME_THRESHOLD && PRINT_TIME_LOGS != 0) 
        printf("Rank %d: initial_alternating_sort took %.6f seconds\n", rank, end_time - start_time);
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
                memcpy(send_buff, local_row, sizeof(int) * cols);

                int base_tag = (stage << 8) | step; // Combine stage and step into a unique tag

                // Determine chunk size
                int chunk_count = CHUNK_DIVISOR;
                int chunk_elements = (cols + chunk_count - 1) / chunk_count;
                MPI_Request send_request[chunk_count], recv_request[chunk_count];

                start_time = MPI_Wtime(); // Start timing for the entire send/receive process
                for (int chunk_idx = 0; chunk_idx < chunk_count; chunk_idx++) {
                    int offset = chunk_idx * chunk_elements;
                    int current_chunk_size = (offset + chunk_elements <= cols) ? chunk_elements : cols - offset;

                    if (current_chunk_size > 0) {
                        int send_tag = (chunk_idx << 16) | base_tag;
                        int recv_tag = (chunk_idx << 16) | base_tag;

                        MPI_Isend(send_buff + offset, current_chunk_size, MPI_INT, partner, send_tag, MPI_COMM_WORLD, &send_request[chunk_idx]);
                        MPI_Irecv(received_row + offset, current_chunk_size, MPI_INT, partner, recv_tag, MPI_COMM_WORLD, &recv_request[chunk_idx]);
                    }
                }

                // Wait for each communication to complete and perform pairwise sort
                for (int chunk_idx = 0; chunk_idx < chunk_count; chunk_idx++) {
                    int offset = chunk_idx * chunk_elements;
                    int current_chunk_size = (offset + chunk_elements <= cols) ? chunk_elements : cols - offset;

                    if (current_chunk_size > 0) {
                        MPI_Wait(&recv_request[chunk_idx], MPI_STATUS_IGNORE);
                        pairwise_sort(local_row + offset, received_row + offset, current_chunk_size, (rank < partner) ? is_ascending : !is_ascending);
                    }
                }
                end_time = MPI_Wtime();
                if (end_time - start_time > MIN_TIME_THRESHOLD && PRINT_TIME_LOGS != 0) 
                    printf("Rank %d: Entire receive process and pairwise_sort took %.6f seconds\n", rank, end_time - start_time);

                start_time = MPI_Wtime(); // Start timing for waiting on all sends
                MPI_Waitall(chunk_count, send_request, MPI_STATUS_IGNORE);
                end_time = MPI_Wtime();
                if (end_time - start_time > MIN_TIME_THRESHOLD && PRINT_TIME_LOGS != 0) 
                    printf("Rank %d: Waiting for all sends took %.6f seconds\n", rank, end_time - start_time);
            }
        }

        // Local elbow sort after each stage
        start_time = MPI_Wtime();
        elbow_sort(local_row, cols, is_ascending);
        end_time = MPI_Wtime();
        if (end_time - start_time > MIN_TIME_THRESHOLD && PRINT_TIME_LOGS != 0) 
            printf("Rank %d: elbow_sort for stage %d took %.6f seconds\n", rank, stage, end_time - start_time);
    }

    // Clean up
    free(received_row);
    free(send_buff);
}




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Version 3 (Optimized Communications - tested)
// #define CHUNK_DIVISOR 16  // Number of chunks to split the data into

// void bitonic_sort(int* local_row, int rows, int cols, int rank) {
//     int stages = (int)log2(rows);

//     // Pre-allocate buffer for communications
//     int* received_row = malloc(cols * sizeof(int));
//     int* send_buff = malloc(cols * sizeof(int));
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
//                 memcpy(send_buff, local_row, sizeof(int) * cols);
//                 int base_tag = (stage << 8) | step; // Combine stage and step into a unique tag

//                 // Determine chunk size
//                 int chunk_count = CHUNK_DIVISOR;
//                 int chunk_elements = (cols + chunk_count - 1) / chunk_count;
//                 MPI_Request send_request[chunk_count], recv_request[chunk_count];

//                 for (int chunk_idx = 0; chunk_idx < chunk_count; chunk_idx++) {
//                     int offset = chunk_idx * chunk_elements;
//                     int current_chunk_size = (offset + chunk_elements <= cols) ? chunk_elements : cols - offset;

//                     if (current_chunk_size > 0) {
//                         int send_tag = (chunk_idx << 16) | base_tag;
//                         int recv_tag = (chunk_idx << 16) | base_tag;

//                         MPI_Isend(send_buff + offset, current_chunk_size, MPI_INT, partner, send_tag, MPI_COMM_WORLD, &send_request[chunk_idx]);
//                         MPI_Irecv(received_row + offset, current_chunk_size, MPI_INT, partner, recv_tag, MPI_COMM_WORLD, &recv_request[chunk_idx]);
//                     }
//                 }

//                 // Wait for communications to complete and perform pairwise sort
//                 for (int chunk_idx = 0; chunk_idx < chunk_count; chunk_idx++) {
//                     int offset = chunk_idx * chunk_elements;
//                     int current_chunk_size = (offset + chunk_elements <= cols) ? chunk_elements : cols - offset;

//                     if (current_chunk_size > 0) {
//                         MPI_Wait(&recv_request[chunk_idx], MPI_STATUS_IGNORE);
//                         pairwise_sort(local_row + offset, received_row + offset, current_chunk_size, (rank < partner) ? is_ascending : !is_ascending);
//                     }
//                 }

//                 MPI_Waitall(chunk_count, send_request, MPI_STATUS_IGNORE);
//             }
//         }

//         // Local elbow sort after each stage
//         elbow_sort(local_row, cols, is_ascending);
//     }

//     // Clean up
//     free(received_row);
// }




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Version 2 (Communication with chunks - tested)
// #define CHUNK_SIZE 8388608  // `2^23`

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
//                 // Generate a unique tag for this stage and step
//                 int base_tag = (stage << 8) | step; // Combine stage and step into a unique tag
//                 int num_of_splitted_chunks = cols / CHUNK_SIZE == 0 ? 1 : cols / CHUNK_SIZE;
//                 MPI_Request send_request[num_of_splitted_chunks], 
//                             recv_request[num_of_splitted_chunks];

//                 int* send_buff = malloc(sizeof(int) * cols);
//                 memcpy(send_buff, local_row, sizeof(int) * cols);

//                 for (int offset = 0; offset < cols; offset += CHUNK_SIZE) {
//                     int current_chunk_size = (offset + CHUNK_SIZE <= cols) ? CHUNK_SIZE : cols - offset;
//                     int send_tag = base_tag + offset; // Adjust tag by offset for chunk
//                     int recv_tag = base_tag + offset;

//                     MPI_Isend(send_buff  + offset, current_chunk_size, MPI_INT, partner, send_tag, MPI_COMM_WORLD, &send_request[offset / CHUNK_SIZE]);
//                     MPI_Irecv(received_row + offset, current_chunk_size, MPI_INT, partner, recv_tag, MPI_COMM_WORLD, &recv_request[offset / CHUNK_SIZE]);
//                 }

//                 for (int offset = 0; offset < cols; offset += CHUNK_SIZE) {
//                     int current_chunk_size = (offset + CHUNK_SIZE <= cols) ? CHUNK_SIZE : cols - offset;
//                     MPI_Wait(&recv_request[offset / CHUNK_SIZE], MPI_STATUS_IGNORE);
//                     pairwise_sort(local_row + offset, received_row + offset, current_chunk_size, (rank < partner) ? is_ascending : !is_ascending);
//                 }

//                 MPI_Waitall(num_of_splitted_chunks, send_request, MPI_STATUS_IGNORE);
//                 free(send_buff);
//             }

//             // // Uncomment if needed for debugging synchronization issues
//             // MPI_Barrier(MPI_COMM_WORLD);
//         }

//         elbow_sort(local_row, cols, is_ascending);
//         // // Uncomment if needed for debugging synchronization issues
//         // MPI_Barrier(MPI_COMM_WORLD);
//     }

//     // Clean up
//     free(received_row);
// }




////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// // Version 1 (simple implementation - tested)
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
