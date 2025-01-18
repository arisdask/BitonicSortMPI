#ifndef VALIDATION_H
#define VALIDATION_H

#include <stdbool.h>
#include <stdio.h>
#include <mpi.h>


/**
 * Checks if the given array (row) is sorted in either ascending or descending order localy.
 *
 * @param row           Pointer to the array to be checked.
 * @param cols          Number of elements in the array.
 * @param is_ascending  Pointer to a bool variable that will store whether the array
 *                      is sorted in ascending order (true) or descending order (false),
 *                      if it is sorted.
 * @return              true if the array is sorted in ascending or descending order,
 *                      false otherwise.
 */
bool is_localy_sorted(int* row, int cols, bool* is_ascending);


/**
 * Validates the correctness of the distributed bitonic sort.
 * Each process checks if its local row is sorted and if the global order is maintained.
 *
 * @param local_row  Pointer to the local row (array of integers).
 * @param cols       Number of columns in each row.
 * @param rank       The rank of the process.
 * @param size       Total number of processes.
 * @param eval       true if this row is sorted with the previous one,
 *                   false otherwise.
 */
void validate_bitonic_sort(int* local_row, int cols, int rank, int size, bool* eval);

#endif