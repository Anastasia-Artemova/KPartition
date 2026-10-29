## Approach

The algorithm combines **greedy initialization** with **single-element swap local search**.

**1. Greedy initialization**
- Sort the elements in non-increasing order.
- Assign each element to the subset with the smallest current sum.

**2. Local search**
- Consider every pair of subsets with different sums.
- Try exchanging every possible pair of elements between them.
- Accept a swap only if it strictly decreases the difference between the maximum and minimum subset sums:

\[
D = \max_i(S_i) - \min_i(S_i)
\]

- Repeat until no improving swap exists, \(D=0\), or the maximum number of iterations is reached.

To guarantee polynomial running time, the number of iterations is limited to:

\[
I_{\max}=n^2
\]

## Time Complexity

- **Greedy initialization:** \(O(n\log n+nk)\)
- **One local-search iteration:** \(O(n^2k)\)
- **Maximum iterations:** \(O(n^2)\)

Therefore, the overall worst-case time complexity is:

\[
\boxed{O(n\log n+n^4k)}
\]

## Compilation and Execution

**Requirements:** C++17-compatible compiler (e.g., GCC).

Compile:

```bash
g++ -std=c++17 -O2 k_partitions_version_1.cpp -o solution
```

Run:

```bash
./solution input/AaC_example_1.txt 4
```

On Windows PowerShell:

```powershell
.\solution.exe input\AaC_example_1.txt 4
```

The program takes two command-line arguments:
1. Path to the input `.txt` file.
2. Number of subsets `k`.

### Input Format

The first line specifies the number of sets. Each following line contains one set of positive integers separated by spaces.

Example:

```text
2
25 20 18 15 12 10 8 7 5
8 7 6 5 4
```

### Output Format

Results are saved to the `output/` directory as `<input_name>_output.txt`.

The output includes the number of sets and `k`, the original elements in input order, each subset and its sum, and the final maximum difference.

Elements within subsets are sorted in non-increasing order, and subsets are sorted lexicographically in non-increasing order.