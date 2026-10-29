
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <climits>
#include <utility>
#include <typeinfo>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <stdexcept>

using namespace std;

class Solution {
private:
    int k;
    vector<int> nums;
    vector<vector<int>> result;
    vector<long long> sums;

    struct Subset {
        vector<int> indices;
        long long sum = 0;
    };

    struct SwapCandidate {
        int groupA = -1;
        int groupB = -1;

        vector<int> subsetA;
        vector<int> subsetB;

        long long deltaEnergy = 0;
    };

    void findContainerWithLeastSum(int elToAdd) {
        int minIdx = 0;

        for (int i = 1; i < k; i++) {
            if (sums[i] < sums[minIdx]) {
                minIdx = i;
            }
        }

        result[minIdx].push_back(elToAdd);
        sums[minIdx] += elToAdd;
    }

    long long getDifference() const {
        auto [minIt, maxIt] = minmax_element(sums.begin(), sums.end());

        return *maxIt - *minIt;
    }

    long long calculateEnergy() const {
        long long energy = 0;

        for (long long sum : sums) {
            energy += sum * sum;
        }

        return energy;
    }

    void helper(vector<Subset>& allSubsets, Subset& currentSubset, int index,
         const vector<int>& container) const {
        if (index == static_cast<int>(container.size())) {
            allSubsets.push_back(currentSubset);
            return;
        }

        currentSubset.indices.push_back(index);
        currentSubset.sum += container[index];

        helper(allSubsets, currentSubset, index + 1, container);

        currentSubset.indices.pop_back();
        currentSubset.sum -= container[index];

        helper(allSubsets, currentSubset, index + 1, container);
    }

    vector<Subset> allSubsetsForOneContainer(int containerIdx) const {
        vector<Subset> subsets;
        Subset currentSubset;

        helper(subsets, currentSubset, 0, result[containerIdx]);

        return subsets;
    }

    long long evaluateSwap(int a, int b, long long transfer) const {
        return 2LL * transfer * (transfer - (sums[a] - sums[b]));
    }

    vector<int> extractSubset(int containerIdx, const vector<int>& indices) {
        vector<int> selected;
        vector<int> remaining;

        const auto& container = result[containerIdx];

        vector<bool> chosen(container.size(), false);

        for (int idx : indices) {
            chosen[idx] = true;
        }

        for (int i = 0; i < static_cast<int>(container.size()); i++) {
            if (chosen[i]) {
                selected.push_back(container[i]);
            } else {
                remaining.push_back(container[i]);
            }
        }

        result[containerIdx] = move(remaining);

        return selected;
    }

    void performSwap(const SwapCandidate& candidate) {
        int a = candidate.groupA;
        int b = candidate.groupB;

        vector<int> fromA = extractSubset(a, candidate.subsetA);

        vector<int> fromB = extractSubset(b, candidate.subsetB);

        long long sumA = accumulate(fromA.begin(), fromA.end(), 0LL);
        long long sumB = accumulate(fromB.begin(), fromB.end(), 0LL);

        result[a].insert(result[a].end(), fromB.begin(), fromB.end());
        result[b].insert(result[b].end(), fromA.begin(), fromA.end());

        long long transfer = sumA - sumB;

        sums[a] -= transfer;
        sums[b] += transfer;
    }

public:
    Solution(vector<int> numbers, int groups)
        : k(groups), nums(move(numbers)), result(groups), sums(groups, 0) {}

    void solve() {
        sort(nums.rbegin(), nums.rend());

        for (int el : nums) {
            findContainerWithLeastSum(el);
        }

        cout << "\nInitial greedy partition:\n";
        printResult();

        vector<vector<int>> bestResult = result;
        vector<long long> bestSums = sums;

        long long bestDifference = getDifference();
        long long bestEnergy = calculateEnergy();

        int iteration = 0;

        while (true) {
            if (bestDifference == 0) {
                break;
            }

            SwapCandidate best;
            long long bestDelta = 0;

            for (int a = 0; a < k; a++) {
                for (int b = a + 1; b < k; b++) {

                    if (sums[a] == sums[b]) {
                        continue;
                    }

                    vector<Subset> subsetsA = allSubsetsForOneContainer(a);
                    vector<Subset> subsetsB = allSubsetsForOneContainer(b);

                    int sizeA = static_cast<int>(result[a].size());
                    int sizeB = static_cast<int>(result[b].size());

                    for (const auto& subsetA : subsetsA) {
                        for (const auto& subsetB : subsetsB) {

                            int newSizeA = sizeA - static_cast<int>(subsetA.indices.size())
                                + static_cast<int>(subsetB.indices.size());

                            int newSizeB = sizeB - static_cast<int>(subsetB.indices.size())
                                + static_cast<int>(subsetA.indices.size());

                            if (newSizeA == 0 || newSizeB == 0) {
                                continue;
                            }

                            long long transfer = subsetA.sum - subsetB.sum;

                            if (transfer == 0) {
                                continue;
                            }

                            long long delta = evaluateSwap(a, b, transfer);

                            if (delta < bestDelta) {
                                bestDelta = delta;

                                best.groupA = a;
                                best.groupB = b;

                                best.subsetA = subsetA.indices;
                                best.subsetB = subsetB.indices;

                                best.deltaEnergy = delta;
                            }
                        }
                    }
                }
            }

            if (best.groupA == -1) {
                break;
            }

            long long oldEnergy = calculateEnergy();
            long long oldDifference = getDifference();

            performSwap(best);

            iteration++;

            long long newEnergy = calculateEnergy();
            long long newDifference = getDifference();

            cout << "\nIteration " << iteration << "\n";

            cout << "Energy: " << oldEnergy << " -> " << newEnergy
                 << " (change = " << best.deltaEnergy << ")\n";

            cout << "Difference: " << oldDifference << " -> " << newDifference << "\n";

            printResult();

            if (newDifference < bestDifference ||
                (newDifference == bestDifference && newEnergy < bestEnergy)) {

                bestDifference = newDifference;
                bestEnergy = newEnergy;

                bestResult = result;
                bestSums = sums;
            }
        }

        result = move(bestResult);
        sums = move(bestSums);

        cout << "\n================================\n";
        cout << "Best partition found:\n";
        printResult();

        cout << "Total improving iterations: " << iteration << "\n";

        cout << "Final difference: " << getDifference() << "\n";
    }

    void printResult() const {
        for (int i = 0; i < k; i++) {
            cout << "Group " << i + 1 << ": ";

            for (int el : result[i]) {
                cout << el << " ";
            }

            cout << "| Sum = " << sums[i] << "\n";
        }

        cout << "------------------------\n";
    }

    vector<int>& getNums() {
        return nums;
    }

    vector<vector<int>>& getResult() {
        return result;
    }
};

void writeNumbers(ostream& out, const vector<int>& numbers) {
    for (size_t i = 0; i < numbers.size(); ++i) {
        if (i > 0) {
            out << " ";
        }
        out << numbers[i];
    }
}

void writeSolution(ostream& out, const vector<int>& originalSet, const vector<vector<int>>& result) {
    long long totalSum = accumulate(originalSet.begin(), originalSet.end(), 0LL);

    out << totalSum << " ";
    writeNumbers(out, originalSet);
    out << "\n";

    vector<vector<int>> sortedGroups = result;

    for (auto& group : sortedGroups) {
        sort(group.rbegin(), group.rend());
    }

    sort(sortedGroups.begin(), sortedGroups.end(), greater<vector<int>>());

    long long minSum = LLONG_MAX;
    long long maxSum = LLONG_MIN;

    for (const auto& group : sortedGroups) {
        long long groupSum = accumulate(group.begin(), group.end(), 0LL);

        minSum = min(minSum, groupSum);
        maxSum = max(maxSum, groupSum);

        out << groupSum << " ";
        writeNumbers(out, group);
        out << "\n";
    }

    out << maxSum - minSum << "\n";
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Please provide the following arguments: <input_file.txt> <k>\n";
        return 1;
    }

    int k;

    try {
        string kArgument = argv[2];
        size_t parsedCharacters = 0;

        k = stoi(kArgument, &parsedCharacters);

        if (parsedCharacters != kArgument.size() || k <= 0) {
            throw invalid_argument("Invalid k");
        }
    } catch (const exception&) {
        cerr << "Invalid k: must be a positive integer.\n";
        return 1;
    }

    namespace fs = std::filesystem;

    fs::path inputPath(argv[1]);

    if (inputPath.extension() != ".txt") {
        cerr << "Input file must have a .txt extension.\n";
        return 1;
    }

    ifstream inputFile(inputPath);

    if (!inputFile.is_open()) {
        cerr << "Failed to open input file.\n";
        return 1;
    }

    int numberOfSets;

    if (!(inputFile >> numberOfSets) || numberOfSets <= 0) {
        cerr << "Invalid number of sets.\n";
        return 1;
    }

    string line;
    getline(inputFile, line); 

    vector<vector<int>> sets;
    vector<Solution> solutions;

    for (int i = 0; i < numberOfSets; ++i) {
        if (!getline(inputFile, line)) {
            cerr << "Missing set " << i + 1 << "\n";
            return 1;
        }

        stringstream ss(line);
        vector<int> numbers;
        int value;

        while (ss >> value) {
            if (value <= 0) {
                cerr << "All elements must be positive.\n";
                return 1;
            }

            numbers.push_back(value);
        }

        if (!ss.eof()) {
            cerr << "Invalid element in set " << i + 1 << "\n";
            return 1;
        }

        if (static_cast<int>(numbers.size()) < k) {
            cerr << "Set " << i + 1 << " has fewer than k elements.\n";
            return 1;
        }

        sets.push_back(numbers);

        Solution solution(numbers, k);
        solution.solve();

        solutions.push_back(move(solution));
    }

    fs::path outputDir = "./output";
    fs::create_directories(outputDir);

    fs::path outputPath = outputDir / (inputPath.stem().string() + "_output.txt");

    ofstream outputFile(outputPath);

    if (!outputFile.is_open()) {
        cerr << "Failed to create output file.\n";
        return 1;
    }

    outputFile << numberOfSets << " " << k << "\n";

    for (int i = 0; i < numberOfSets; ++i) {
        outputFile << "\n";

        writeSolution(outputFile, sets[i], solutions[i].getResult());
    }

    if (!outputFile) {
        cerr << "Error writing output file.\n";
        return 1;
    }

    return 0;
}

