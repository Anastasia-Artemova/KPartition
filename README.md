# K-Partition Problem — Greedy and Energy-Based Local Search

## Problem Description

The goal is to partition a set of positive integers into **k non-empty subsets**, minimizing the difference between the largest and smallest subset sums:

\[
D = \max_i(S_i) - \min_i(S_i)
\]

where \(S_i\) is the sum of elements in subset \(i\).

## Approach

Our solution combines two heuristics:

**1. Greedy initialization**

- Sort the input numbers in non-increasing order.
- Assign each number to the subset with the currently smallest sum.
- This produces an initial, reasonably balanced partition.

**2. Energy-based local search**

We define the energy function as:

\[
E = \sum_{i=1}^{k} S_i^2
\]

Minimizing this function encourages balanced subset sums.

At each iteration:

- Consider every pair of subsets.
- Enumerate all possible subset exchanges between them, including one-way transfers.
- Calculate the energy change without modifying the current partition:

\[
\Delta E = 2d(d-(S_a-S_b))
\]

where \(d\) is the difference between the sums of the exchanged elements.

- Perform the exchange with the largest energy decrease (\(\Delta E < 0\)).
- Repeat until no improving exchange exists.

Throughout the search, we retain the best partition according to the smallest maximum difference \(D\), using energy as a tie-breaker.

The algorithm is heuristic, meaning it does not guarantee a globally optimal partition.

## Approximate Time Complexity

Let \(n\) be the number of elements, \(k\) the number of subsets, and \(m_i\) the size of subset \(i\).

- **Greedy initialization:** \(O(n\log n + nk)\).
- **Local search:** For each pair of subsets, approximately \(O(2^{m_i+m_j})\) subset exchanges are examined.
- **Overall:** Approximately \(O(Ik^2 2^{2n/k})\) for reasonably balanced subset sizes, where \(I\) is the number of local-search iterations.

## Compilation and Execution

**Requirements:** C++17-compatible compiler (e.g., GCC).

Compile:

```bash
g++ -std=c++17 -O2 k_partitions.cpp -o solution
```

Run:

```bash
./solution input/AaC_example_1.txt 4
```

On Windows PowerShell:

```powershell
.\solution.exe input\AaC_example_1.txt 4
```

The program accepts two command-line arguments:

1. Path to the input `.txt` file.
2. Number of subsets `k`.

### Input Format

The first line specifies the number of sets. Each subsequent line contains one set of positive integers separated by spaces.

Example:

```text
2
25 20 18 15 12 10 8 7 5
8 7 6 5 4
```

### Output Format

Results are saved in the `output/` directory under the filename `<input_name>_output.txt`.

The output contains the number of sets and `k`, followed by each original set, its resulting subsets and their sums, and the maximum difference. Numbers within subsets are sorted in descending order, and subsets are sorted lexicographically in descending order. Consecutive sets are separated by a blank line.