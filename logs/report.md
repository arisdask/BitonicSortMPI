## Sorting Time Report

Running our implementation for `q = [20, 23, 26, 27]` and `p = [1:7]`, we present our results in the following table (time in `msec`):

| $q$    | $p = 1$     | $p = 2$     | $p = 3$     | $p = 4$     | $p = 5$     | $p = 6$     | $p = 7$     |
|--------|-------------|-------------|-------------|-------------|-------------|-------------|-------------|
| 20     | 147.22      | 199.52      | 251.96      | 245.8       | 429         | 752         | 1675.08     |
| 23     | 847.08      | 1129.44     | 2116.39     | 2508.76     | 4236.04     | 7735.43     | 13594.65    |
| 26     | 10802.45    | 14403.27    | 17777.00    | 25605.68    | 41827.59    | 79570.32    | 110331.75   |
| 27     | 25238.01    | 33650.67    | 42136.52    | 55542.93    | 89735.39    | 173050.67   | 221723.30   |

> **Note:**
> The time measurements presented here are based on the current traffic of the Aristotelis HPC system at the time our tests were conducted. This traffic can influence the overall sorting time by affecting the communication speed (making it faster or slower). Results may vary under different network conditions.

When running this implementation, we adjust the `CHUNK_DIVISOR` parameter in the `src/bitonic_sort.c` file. *Fine-tuning this variable based on the current network traffic may lead to better results.*

To ***print all traffic logs***, enable the `PRINT_TIME_LOGS` option in the `src/bitonic_sort.c` file.
