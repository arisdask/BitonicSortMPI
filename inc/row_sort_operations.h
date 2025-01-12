#ifndef ROW_SORT_OPERATIONS_H
#define ROW_SORT_OPERATIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


/**
 * Performs elementwise comparison and swap between two rows
 * 
 * @param row1       First row for comparison
 * @param row2       Second row for comparison
 * @param cols       Number of elements in each row
 * @param ascending  If true, ensure row1[i] <= row2[i]; if false, ensure row1[i] >= row2[i]
 */
void pairwise_sort(int* row1, int* row2, int cols, bool ascending);


/**
 * Performs sorting within a single row using the elbow pattern
 * 
 * @param row        Array representing the row to be sorted
 * @param cols       Number of elements in the row
 * @param ascending  If true, sort in ascending order; if false, sort in descending order
 */
void elbow_sort(int* row, int cols, bool ascending);


#endif
