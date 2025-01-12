#include <../inc/row_sort_operations.h>


// Elementwise sort between two rows
void pairwise_sort(int* row1, int* row2, int cols, bool ascending) {
    for (int j = 0; j < cols; j++) {
        if (ascending) {
            if (row1[j] > row2[j]) {
                int temp = row1[j];
                row1[j] = row2[j];
                row2[j] = temp;
            }
        } else {
            if (row1[j] < row2[j]) {
                int temp = row1[j];
                row1[j] = row2[j];
                row2[j] = temp;
            }
        }
    }
}


// Elbow sort for a single row
void elbow_sort(int* row, int cols, bool ascending) {
    if (ascending) {
        for (int i = 0; i < cols - 1; i++) {
            for (int j = i + 1; j < cols; j++) {
                if (row[i] > row[j]) {
                    int temp = row[i];
                    row[i] = row[j];
                    row[j] = temp;
                }
            }
        }
    } else {
        for (int i = 0; i < cols - 1; i++) {
            for (int j = i + 1; j < cols; j++) {
                if (row[i] < row[j]) {
                    int temp = row[i];
                    row[i] = row[j];
                    row[j] = temp;
                }
            }
        }
    }
}
