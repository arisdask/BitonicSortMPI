### A Pluto.jl notebook ###
# v0.20.3

using Markdown
using InteractiveUtils

# ╔═╡ 24ca78c0-bd47-11ef-0ce4-2b4b3bdabad3
md"
# Hybrid Distributed Sort with Bitonic Interchanges

> _**Author:** Aristeidis Daskalopoulos_


In this notebook, we introduce an implementation of the Bitonic Sort algorithm, specifically designed to run optimally on parallel computing architectures using the Message Passing Interface (MPI).

The main objective of this notebook is to provide a high-level overview of the algorithm, explaining the initial implementation of distributed bitonic sort in a sequential manner. We will explore a matrix-based analogy to facilitate the sequential execution of the code.

To view the complete distributed implementation of the code in C, please refer to the `inc` and `src` directories in the [repository](https://github.com/arisdask/BitonicSortMPI).
"


# ╔═╡ ce6047c8-f0a6-436b-916d-2dab9088413b
md"
### **Bitonic Sort - Basic Functionality**

We will first demonstrate an example of how the Bitonic Sort works across multiple processes, which need to communicate with each other. 

Let’s assume we have 8 processes:

"


# ╔═╡ d4d39ad3-2f33-4d7d-936f-ba3c27908cf1
md"""
![bitonic-network](bitonic-network.png)
"""


# ╔═╡ 595c2aa5-16c5-4156-be28-c2ee94cb0fad
md"
As the diagram shows:

1. **Local Sort (Initialization):**
   - The array is divided among 8 processes.
   - Each process sorts its local data independently.

2. **Communication Phase:**
   - Processes perform **element-wise comparisons**:
     - Exchange `min` and `max` values between pairs of processes.
   - This creates **bitonic sequences**.

3. **Bitonic Merge:**
   - In multiple rounds, processes:
     - Merge adjacent bitonic sequences.
     - Sort locally (with elbow sort) within each process.

4. **Recursive Merging:**
   - Larger bitonic sequences are formed and sorted at each step.
   - Communication and local sorting continue until the entire array is sorted.

5. **Final Result:**
   - The array becomes **fully sorted** through collaborative merging.

✔ **Parallel Execution**: Local sorting + inter-process communication = efficient sorting.

"

# ╔═╡ ef36ee0c-0669-4d69-b2f4-b19abd85c606
md"""
## **Explanation of Functions**

### **1. `elementwiseSort!(A::Matrix{Int}, i::Int, j::Int, isAscending::Bool)`**

**Purpose**:  
Performs a **compare-and-swap** operation between two rows of a matrix.

- **Input**: 
   - `A`: A matrix of integers.
   - `i`, `j`: Indices of the rows to compare.
   - `isAscending`: A boolean to determine sorting order.

- **Functionality**:
   - If `isAscending` is true: 
     - Row `i` gets the **minimum** values of `row1` and `row2`.
     - Row `j` gets the **maximum** values.
   - If `isAscending` is false:
     - Row `i` gets the **maximum** values.
     - Row `j` gets the **minimum** values.

**Comment**:  
This function is essential for the **compare-and-swap** step in the Bitonic Sort algorithm.
"""


# ╔═╡ adf107b6-2241-47dc-992d-f5d611e8eaa5
# Perform the bitonic compare-and-swap operations
function elementwiseSort!(A::Matrix{Int}, i::Int, j::Int, isAscending::Bool)
    row1 = A[i, :]
    row2 = A[j, :]
    if isAscending
        A[i, :] .= min.(row1, row2)
        A[j, :] .= max.(row1, row2)
    else
        A[i, :] .= max.(row1, row2)
        A[j, :] .= min.(row1, row2)
    end
end

# ╔═╡ 5b77e273-6ca0-4bde-8a3a-00582d349704
md"""
### **2. `findElbow(arr::AbstractVector{T}) where T`**

**Purpose**:  
Identifies the **elbow point** of a bitonic sequence, where the order changes (from ascending to descending or vice versa).

- **Input**:
   - `arr`: A vector of elements.

- **Functionality**:
   - Checks for two cases:
     1. Descending-to-ascending (indicating the minimum).
     2. Ascending-to-descending (indicating the maximum).
   - Iterates over the array to locate the "elbow" where order breaks.
   - Returns:
     - `i`: Index of the elbow point.
     - `elbowFlag`: 1 if elbow is a **minimum**, 2 if elbow is a **maximum**.

**Comment**:  
This function helps in identifying the transition point in a bitonic sequence, a critical step in sorting bitonic data.
"""


# ╔═╡ 1c377437-76b5-4e4f-80d2-3c63cc0abbd5
# function findElbow(arr::Vector{T}) where T
function findElbow(arr::AbstractVector{T}) where T
    if length(arr) < 3
        return 1, 0
    end

    elbowFlag = 0
    
    # Wrapper to check both descending-to-ascending and ascending-to-descending to find the elbow
    for orderCheck in [
        (j -> arr[j] > arr[j+1]),   # Check descending first
        (j -> arr[j] < arr[j+1])    # Then check ascending
    ]
        elbowFlag = elbowFlag + 1  # If 1: the elbow indicates the min of the list (If 2: is at the max)
        for i in 2:length(arr)-1
            if all( (orderCheck(j) || arr[j] == arr[j+1]) for j in 1:i-1 ) && !(orderCheck(i) || arr[i] == arr[i+1])
                if all( (arr[j] == arr[j+1]) for j in 1:i-1 )
                    continue
                end
                return i, elbowFlag
            end
        end
    end
    
    # No elbow point found (array already sorted or non-bitonic)
    return 1, 0
end

# ╔═╡ 88ae0603-686e-402a-9b38-0b2647bc631b
md"""
### **3. `elbowSort!(arr::AbstractVector{T}, isAscending::Bool) where T`**

**Purpose**:  
Sorts a bitonic sequence based on the elbow point detected.

- **Input**:
   - `arr`: A vector to be sorted.
   - `isAscending`: Determines if sorting is ascending or descending.

- **Functionality**:
   1. Calls `findElbow` to detect the elbow point.
   2. If the array is already sorted or non-bitonic, it directly returns the sorted version.
   3. Sorts the array by:
      - Iteratively placing elements from the two sides of the elbow into a result vector.
   4. Handles wrapping around at boundaries (circular array logic).

- **Edge Cases**:
   - If no elbow is found, the array is returned as is.
   - Handles already sorted or reversed arrays.

**Comment**:  
This function sorts the bitonic sequence efficiently based on the elbow, enabling its use in the merging step of Bitonic Sort.
"""


# ╔═╡ b5f37cc6-b329-4bb6-88fd-7e323ec4716a
# function elbowSort!(arr::Vector{T}, isAscending::Bool) where T
function elbowSort!(arr::AbstractVector{T}, isAscending::Bool) where T
    # Find the elbow indices
    i, elbowFlag = findElbow(arr)
    j = i + 1
    
    # If no elbow found, the array is already sorted (or non-bitonic)
    if i == 1 && elbowFlag == 0
        if isAscending && all(arr[k] <= arr[k+1] for k in eachindex(arr)[1:end-1]) ||
           (!isAscending && all(arr[k] >= arr[k+1] for k in eachindex(arr)[1:end-1]))
            return arr
        elseif (isAscending && all(arr[k] >= arr[k+1] for k in eachindex(arr)[1:end-1])) ||
               (!isAscending && all(arr[k] <= arr[k+1] for k in eachindex(arr)[1:end-1]))
            return reverse!(arr)
        else
            return arr
        end
    end
    

    res = Vector{T}(undef, length(arr))
    cnt = 1
    
    # Ascending Order Sorting
    while cnt <= length(res)
        # For ascending order: put largest value first IF the elbow indicates the min
        if elbowFlag == 1
            if i == j
                res[cnt] = arr[i]
                cnt = cnt + 1
                break
            end

            if arr[i] > arr[j]
                res[cnt] = arr[j]
                cnt = cnt + 1
                j = j + 1
            elseif arr[i] < arr[j]
                res[cnt] = arr[i]
                cnt = cnt + 1
                i = i - 1
            else 
                res[cnt] = arr[i]
                cnt = cnt + 1
                i = i - 1
                res[cnt] = arr[j]
                cnt = cnt + 1
                j = j + 1
            end

        elseif elbowFlag == 2
            if i == j
                res[length(res) - cnt + 1] = arr[i]
                cnt = cnt + 1
                break
            end

            if arr[i] > arr[j]
                res[length(res) - cnt + 1] = arr[i]
                cnt = cnt + 1
                i = i - 1
            elseif arr[i] < arr[j]
                res[length(res) - cnt + 1] = arr[j]
                cnt = cnt + 1
                j = j + 1
            else 
                res[length(res) - cnt + 1] = arr[i]
                cnt = cnt + 1
                i = i - 1
                res[length(res) - cnt + 1] = arr[j]
                cnt = cnt + 1
                j = j + 1
            end
        end

        # Boundary checks
        if i < 1
            # Reached the start of the array
            i = length(arr)
        end
        if j > length(arr)
            # Reached the end of the array
            j = 1
        end
    end

    if !isAscending
        reverse!(res)
    end

    arr .= res
    return arr
end

# ╔═╡ 9a50e0a7-f04a-4289-803e-421d6bdb18e6
md"""
## Understanding the Bitonic Sort Implementation

The `bitonicSort!` function is a parallel sorting algorithm designed to sort a matrix in-place with specific sorting characteristics. Let's break down its key components:

### Initial Setup and Preprocessing

1. **Matrix Dimensions**:
   - The function assumes the input matrix has rows with a number of elements that is a power of 2.
   - `p` represents the number of rows in the matrix
   - `q` calculates the number of sorting stages using log2

### Step 1: Initial Alternating Sort
- Rows are initially sorted with an alternating pattern:
  - Odd-indexed rows are sorted in ascending order
  - Even-indexed rows are sorted in descending order
- This creates an initial "bitonic" sequence where the rows are partially sorted with alternating directions

### Step 2: Iterative Bitonic Merging
The core of the algorithm involves multiple stages of sorting:
- Iterates through `stages` (determined by log2 of matrix rows)
- In each stage:
  - Divides the matrix into chunks
  - Determines sorting direction for each chunk
  - Performs recursive element-wise comparisons and swaps
  - Uses a complex nested loop structure to manage comparisons across different scales
  - *Note*: This implementation serves only to emulate the parallel approach. The actual parallel version described [here](https://github.com/arisdask/BitonicSortMPI/blob/main/BitonicSortMPI.pdf) implements a more user-friendly algorithm.

### Final Stage: Elbow Sort
- Applies an additional sorting pass with a predefined pattern
- Uses a binary pattern to determine sorting direction for each row
- Helps to further refine the sorting of the matrix

The algorithm is particularly interesting because it:
- Works in-place
- Has a parallel-friendly structure
- Sorts both rows and elements within rows
- Uses a logarithmic number of stages to complete sorting

### Key Helper Functions
The implementation relies on two critical helper functions:
- `elementwiseSort!`: Compares and potentially swaps elements
- `elbowSort!`: Applies a specialized sorting technique on row views
"""


# ╔═╡ 80620c6b-a212-45a1-ba89-4bd64572c10f
# Perform the iterative bitonic sort
function bitonicSort!(A::Matrix{Int})
    p = size(A, 1)  # Number of rows (assumes n is a power of 2)
    q = Int(log2(p))  # Number of stages

    # Step 1: Initial alternating sorting
    for i in 1:1:p
        if iseven(i)
            A[i, :] .= sort(A[i, :], rev=true)  # Descending for even rows
        else
            A[i, :] .= sort(A[i, :])            # Ascending for odd rows
        end
    end

    # Step 2: Iterative bitonic merging
    stages = q
    for stage in 1:1:stages
        numOfChunks = 2^(stages - stage)
        sizeOfChunk = Int(p / numOfChunks)

        for chunk in 1:1:numOfChunks
            isAscending = true
            if chunk % 2 == 0
                isAscending = false
            end

            recursiveStep = 2^(stage - 1)
            counter = 0
            while recursiveStep >= 1
                numOfCopies = 2^counter
                for k in 1:1:( (sizeOfChunk >> 1) >> counter )
                    i = k + (chunk - 1) * sizeOfChunk

                    for c in 1:1:numOfCopies
                        j = (c - 1) * (sizeOfChunk >> counter)
                        elementwiseSort!(A, i + j, i + j + recursiveStep, isAscending)
                        
                        # print("Stage = ", stage, ": ", i + j, "-", i + j + recursiveStep)
                        # if isAscending
                        #     println(" (ascending)")
                        # else
                        #     println(" (descending)")
                        # end
                    end
                end
                recursiveStep = recursiveStep >> 1
                counter += 1
            end
        end

        # Elbow sort at each iteration:
        pattern = vcat(zeros(Int, 2^stage), ones(Int, 2^stage))
        rev = repeat(pattern, ceil(Int, p / length(pattern)))[1:p]
        for pid in 1:1:p 
            elbowSort!(view(A, pid, :), !Bool(rev[pid]))
        end
    end
end

# ╔═╡ de71b8d3-0e47-4689-a42d-cd1275b3213d
md"""
## Test Function

The test function `main()` is designed to validate the correctness of the `bitonicSort!` function through a comprehensive comparison process. Here's a detailed breakdown:

### Test Setup
1. **Matrix Dimensions**
   - `p`: Number of rows (power of 2)
   - `n`: Number of columns (power of 2)
   - `MAX_INT`: Maximum random integer value

### Test Workflow

1. **Random Matrix Generation**
   ```julia
   A = rand(1:MAX_INT, p, n)
   ```
   - Creates a random matrix `A` with integers between 1 and 500

2. **Flattening the Matrix**
   ```julia
   B = Vector{Int}(undef, p * n)
   for i in 1:p 
       for j in 1:n 
           B[(i-1) * n + j] = A[i, j]
       end
   end
   ```
   - Converts the 2D matrix `A` into a 1D vector `B`
   - Uses a linear indexing transformation

3. **Performance Timing**
   ```julia
   @time bitonicSort!(A)  # Sort 2D matrix
   @time B = sort(B)      # Sort 1D vector
   ```
   - Uses `@time` macro to measure sorting time for both matrix and vector
   - Applies bitonic sort to `A`
   - Uses standard Julia `sort()` on `B`

4. **Correctness Verification**
   ```julia
   failFlag = false 
   for i in 1:p 
       for j in 1:n 
           if B[(i-1) * n + j] != A[i, j]
               println("Test Failed at ", A[i, j], "-", B[(i-1) * n + j], " :(")
               failFlag = true
               break
           end
       end
       if failFlag
           display(A)
           display(B)
           break
       end
   end
   ```
   - Compares each element of sorted `A` with sorted `B`
   - If any mismatch is found:
     - Prints failure details
     - Sets `failFlag` to true
     - Displays both matrices
     - Breaks the comparison loop

5. **Test Result**
   ```julia
   if !failFlag
       println("Test PASSED :)")
   end
   ```
   - Prints success message if no mismatches found
"""


# ╔═╡ 674d8967-463b-4206-a3fa-5765e391d56b
# Main test function
function main()
    p = 8  # Number of rows (must be a power of 2)
    n = 128  # Number of columns (must be a power of 2)
    MAX_INT = 500
    A = rand(1:MAX_INT, p, n)
    B = Vector{Int}(undef, p * n)

    for i in 1:1:p 
        for j in 1:1:n 
            B[(i-1) * n + j] = A[i, j]
        end
    end

    @time bitonicSort!(A)
    @time B = sort(B)

    failFlag = false 
    for i in 1:1:p 
        for j in 1:1:n 
            if B[(i-1) * n + j] != A[i, j]
                println("Test Failed at ", A[i, j], "-", B[(i-1) * n + j], " :(")
                failFlag = true
                break
            end
        end

        if failFlag
            display(A)
            display(B)
            break
        end
    end

    if !failFlag
        println("Test PASSED :)")
    end
end

# ╔═╡ b278f440-7584-458c-80d2-19067a3b64bd
main()

# ╔═╡ 6b1cf148-8a08-4469-8dda-ef42847cbf4d
md"""
As anticipated, executing a parallel algorithm sequentially results in slower performance compared to traditional sorting functions. For detailed performance improvements of the distributed implementation written in C, please refer to the [report.md](https://github.com/arisdask/BitonicSortMPI/blob/main/logs/report.md) file.

"""

# ╔═╡ 1376abf1-acfd-475b-b71f-f83e15915cc6
md"""
### Credits

These repository's notebooks and papers were written with writing assistance provided by Claude $3.5$ Sonnet, an AI assistant created by Anthropic.
"""

# ╔═╡ Cell order:
# ╟─24ca78c0-bd47-11ef-0ce4-2b4b3bdabad3
# ╟─ce6047c8-f0a6-436b-916d-2dab9088413b
# ╟─d4d39ad3-2f33-4d7d-936f-ba3c27908cf1
# ╟─595c2aa5-16c5-4156-be28-c2ee94cb0fad
# ╟─ef36ee0c-0669-4d69-b2f4-b19abd85c606
# ╠═adf107b6-2241-47dc-992d-f5d611e8eaa5
# ╟─5b77e273-6ca0-4bde-8a3a-00582d349704
# ╠═1c377437-76b5-4e4f-80d2-3c63cc0abbd5
# ╟─88ae0603-686e-402a-9b38-0b2647bc631b
# ╟─b5f37cc6-b329-4bb6-88fd-7e323ec4716a
# ╟─9a50e0a7-f04a-4289-803e-421d6bdb18e6
# ╠═80620c6b-a212-45a1-ba89-4bd64572c10f
# ╟─de71b8d3-0e47-4689-a42d-cd1275b3213d
# ╠═674d8967-463b-4206-a3fa-5765e391d56b
# ╠═b278f440-7584-458c-80d2-19067a3b64bd
# ╟─6b1cf148-8a08-4469-8dda-ef42847cbf4d
# ╟─1376abf1-acfd-475b-b71f-f83e15915cc6
