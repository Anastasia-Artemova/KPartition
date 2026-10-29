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

public:
    Solution(vector<int> numbers, int groups)
        : k(groups), nums(move(numbers)), result(groups), sums(groups, 0) {}

    void solve() {
        int n = static_cast<int>(nums.size());

        const long long MAX_ITERATIONS = 1LL * n * n;

        sort(nums.rbegin(), nums.rend());

        for (int el : nums) {
            findContainerWithLeastSum(el);
        }

        cout << "Initial greedy partition:\n";
        printResult();

        long long iteration = 0;

        while (iteration < MAX_ITERATIONS) {
            long long currDiff = getDifference();

            if (currDiff == 0) {
                break;
            }

            bool improved = false;

            for (int a = 0; a < k; a++) {
                for (int b = a + 1; b < k; b++) {

                    if (sums[a] == sums[b]) {
                        continue;
                    }

                    for (int i = 0; i < static_cast<int>(result[a].size()); i++) {
                        for (int j = 0; j < static_cast<int>(result[b].size()); j++) {

                            int x = result[a][i];
                            int y = result[b][j];

                            swap(result[a][i], result[b][j]);

                            sums[a] = sums[a] - x + y;
                            sums[b] = sums[b] - y + x;

                            long long newDiff = getDifference();

                            if (newDiff < currDiff) {
                                improved = true;
                                currDiff = newDiff;
                            } else {
                                swap(result[a][i], result[b][j]);

                                sums[a] = sums[a] + x - y;
                                sums[b] = sums[b] + y - x;
                            }
                        }
                    }
                }
            }

            if (!improved) {
                break;
            }

            ++iteration;

            cout << "Iteration " << iteration << ", difference: " << currDiff << "\n";
            printResult();
        }

        cout << "\nBest partition found:\n";
        printResult();

        cout << "Total iterations: " << iteration << "\n";
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

    const vector<vector<int>>& getResult() const {
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
