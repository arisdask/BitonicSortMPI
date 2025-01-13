#include <../inc/row_sort_operations.h>


// Pairwise sort between two rows
// A pair is a [ row1[j], row2[j] ]
void pairwise_sort(int* row1, int* row2, int cols, bool ascending) {
    for (int j = 0; j < cols; j++) {
        if ( (ascending && row1[j] > row2[j]) || (!ascending && row1[j] < row2[j]) ) {
            int temp = row1[j];
            row1[j] = row2[j];
            row2[j] = temp;
        }
    }
}


// Helper function to find the index of min/max(elbow) element in array
int find_elbow_element(const int* arr, int len_arr, bool find_min) {
    int elbow_idx = 0;
    for (int i = 1; i < len_arr; i++) {
        if ( (find_min && arr[i] < arr[elbow_idx]) || (!find_min && arr[i] > arr[elbow_idx]) ) {
            elbow_idx = i;
        }
    }
    return elbow_idx;
}


// Performs sorting within a single row using the elbow pattern
void elbow_sort(int* row, int cols, bool ascending) {
    if (cols <= 1) return;  // Already sorted
    
    // Allocate tmp buffer
    int* tmp_buff = (int*)malloc(cols * sizeof(int));
    if (!tmp_buff) return;  // Handle allocation failure
    
    // Find the elbow point (min/max element)
    int left = find_elbow_element(row, cols, ascending);
    int right = (left == cols - 1) ? 0 : left + 1;
    
    for (int i = 0; i < cols; i++) {
        bool compare;
        if (ascending) {
            compare = row[left] <= row[right];
        } else {
            compare = row[left] >= row[right];
        }
        
        if (compare) {
            tmp_buff[i] = row[left];
            left = (left == 0) ? cols - 1 : left - 1;
        } else {
            tmp_buff[i] = row[right];
            right = (right + 1) % cols;
        }
    }
    
    // Copy tmp_buff buffer back to original array
    memcpy(row, tmp_buff, cols * sizeof(int));
    
    // Clean up
    free(tmp_buff);
}


// // Elbow sort for a single row (version 0: a simple sort)
// void elbow_sort(int* row, int cols, bool ascending) {
//     if (ascending) {
//         for (int i = 0; i < cols - 1; i++) {
//             for (int j = i + 1; j < cols; j++) {
//                 if (row[i] > row[j]) {
//                     int temp = row[i];
//                     row[i] = row[j];
//                     row[j] = temp;
//                 }
//             }
//         }
//     } else {
//         for (int i = 0; i < cols - 1; i++) {
//             for (int j = i + 1; j < cols; j++) {
//                 if (row[i] < row[j]) {
//                     int temp = row[i];
//                     row[i] = row[j];
//                     row[j] = temp;
//                 }
//             }
//         }
//     }
// }
