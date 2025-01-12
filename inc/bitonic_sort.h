#ifndef BITONIC_SORT_H
#define BITONIC_SORT_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <mpi.h>
#include <utils.h>
#include <row_sort_operations.h>


/**
 * Implements the parallel bitonic sort algorithm for a 2D matrix.
 * Each process handles a single row and communicates with the rest as needed.
 * 
 * @param local_row  Array containing the local portion of the matrix (the full array to be sorted)
 * @param rows       Total number of rows(processes) in the complete matrix
 * @param cols       Number of columns in each row
 * @param rank       Rank of the current process
 * 
 */
void bitonic_sort(int* local_row, int rows, int cols, int rank);

#endif
