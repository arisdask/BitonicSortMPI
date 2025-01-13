#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <mpi.h>

// Version 0: serial qsort
// Version 1: multithreaded sorting
#define SORT_VERSION 0


/**
 * Sorts a single row either in ascending or descending order using qsort
 * 
 * @param row        Array representing the row to be sorted
 * @param cols       Number of elements in the row
 * @param ascending  If true, sort in ascending order; if false, sort in descending order
 */
void local_sort(int* row, int cols, bool ascending);


/**
 * Performs the initial alternating sort on the current local row
 * Each row based on its global position/rank is sorted alternately in ascending/descending order
 * 
 * @param row        Array representing the local row to be sorted
 * @param cols       Number of columns in each row
 * @param rank       Rank of the current process
 */
void initial_alternating_sort(int* row, int cols, int rank);


/**
 * Prints the complete row/data of the process.
 * 
 * @param row    Array containing the local portion of the matrix
 * @param cols   Number of columns in the row
 * @param rank   Rank of the current process
 * @param size   Total number of rows/processes
 * 
 */
void print_row(int* row, int cols, int rank, int size) ;

#endif