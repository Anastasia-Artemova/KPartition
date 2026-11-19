#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <climits>
#include <utility>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <stdexcept>
#include <set>
#include <map>

using namespace std;

class Partition {
private:
    vector<long long> sums;
    vector<vector<int>> elements;

public:
    explicit Partition(int k) : sums(k, 0), elements(k) {}

    void addElement(int el, int idx) {
        sums.at(idx) += el;
        elements.at(idx).push_back(el);
    }

    void removeElement(int el, int idx) {
        sums.at(idx) -= el;
        auto& group = elements.at(idx);
        auto it = find(group.begin(), group.end(), el);
        if (it == group.end())  throw logic_error("Element not found");
        group.erase(it);
    }

    long long getCurrDifference() const {
        auto [minIt, maxIt] = minmax_element(sums.begin(), sums.end());
        return *maxIt - *minIt;
    }

    const vector<long long>& getSums() const { return sums; }
    vector<vector<int>> getSubpartitions() const { return elements; }

    
    static vector<Partition> generatePermutations(const Partition& p1, const Partition& p2) {
        vector<Partition> result;

        int k = static_cast<int>(p2.sums.size());

        map<long long, vector<int>> groups;

        for (int i = 0; i < k; ++i) {
            groups[p2.sums[i]].push_back(i);
        }

        vector<long long> values;

        for (const auto& [sum, indices] : groups) {
            for (size_t j = 0; j < indices.size(); ++j) {
                values.push_back(sum);
            }
        }

        set<vector<long long>> seen;

        do {
            Partition temp = p1;

            map<long long, size_t> used;

            for (int i = 0; i < k; ++i) {
                long long sum = values[i];

                size_t position = used[sum]++;
                int originalIndex = groups[sum][position];

                temp.sums[i] += p2.sums[originalIndex];

                const auto& source = p2.elements[originalIndex];

                temp.elements[i].insert(temp.elements[i].end(), source.begin(), source.end());
            }

            vector<long long> canonical = temp.sums;
            sort(canonical.rbegin(), canonical.rend());

            if (seen.insert(canonical).second) {
                result.push_back(move(temp));
            }

        } while (next_permutation(values.begin(), values.end()));

        stable_sort(result.begin(), result.end(),
        [](const Partition& a, const Partition& b) {
            return a.getCurrDifference() <
                b.getCurrDifference();
        });

        return result;
    }

    bool hasEmptySubset() const {
        return any_of(
            elements.begin(),
            elements.end(),
            [](const vector<int>& group) {
                return group.empty();
            }
        );
    }

};

class CKK {
private:
    int k;
    vector<Partition> partitions;
    Partition bestPartition;
    long long bestDiff = LLONG_MAX;

    void recursiveSolver(vector<Partition> current) {
        if (bestDiff == 0) return;
        if (current.size() == 1) {
            if (current[0].hasEmptySubset()) return;

            const long long diff = current[0].getCurrDifference();
            if (diff < bestDiff) {
                bestDiff = diff;
                bestPartition = current[0];
            }
            return;
        }

        stable_sort(current.begin(), current.end(),
                    [](const Partition& a, const Partition& b) {
                        return a.getCurrDifference() > b.getCurrDifference();
                    });

        const auto permutations = Partition::generatePermutations(current[0], current[1]);
        vector<Partition> remaining(current.begin() + 2, current.end());

        for (const Partition& merged : permutations) {
            vector<Partition> next = remaining;
            next.push_back(merged);
            recursiveSolver(move(next));
            if (bestDiff == 0) return;
        }
    }

    void initializePartitions(vector<int> nums) {
        sort(nums.rbegin(), nums.rend());
        for (int num : nums) {
            Partition p(k);
            p.addElement(num, 0);
            partitions.push_back(move(p));
        }
    }

public:
    CKK(int k, vector<int> numbers) : k(k), bestPartition(k) {
        if (k <= 0 || numbers.empty()) throw invalid_argument("Invalid input");
        if (static_cast<int>(numbers.size()) < k)
            throw invalid_argument("Fewer numbers than subsets");
        if (any_of(numbers.begin(), numbers.end(), [](int x) { return x <= 0; }))
            throw invalid_argument("All numbers must be positive");
        initializePartitions(move(numbers));
    }

    pair<long long, Partition> solve() {
        recursiveSolver(partitions);
        return {bestDiff, bestPartition};
    }
};

void writeNumbers(ostream& out, const vector<int>& numbers) {
    for (size_t i = 0; i < numbers.size(); ++i) {
        if (i > 0) out << ' ';
        out << numbers[i];
    }
}

void writeSolution(ostream& out, const vector<int>& originalSet,
                   long long diff, const Partition& partition) {
    const long long totalSum = accumulate(originalSet.begin(), originalSet.end(), 0LL);
    out << totalSum << ' ';
    writeNumbers(out, originalSet);
    out << '\n';

    vector<vector<int>> sortedGroups = partition.getSubpartitions();
    for (auto& group : sortedGroups) sort(group.rbegin(), group.rend());
    sort(sortedGroups.begin(), sortedGroups.end(), greater<vector<int>>());

    for (const auto& group : sortedGroups) {
        const long long groupSum = accumulate(group.begin(), group.end(), 0LL);
        out << groupSum << ' ';
        writeNumbers(out, group);
        out << '\n';
    }
    out << diff << '\n';
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Please provide the following arguments: <input_file.txt> <k>\n";
        return 1;
    }
    int k;
    try {
        const string kArgument = argv[2];
        size_t parsedCharacters = 0;
        k = stoi(kArgument, &parsedCharacters);
        if (parsedCharacters != kArgument.size() || k <= 0)
            throw invalid_argument("Invalid k");
    } catch (const exception&) {
        cerr << "Invalid k: must be a positive integer.\n";
        return 1;
    }

    namespace fs = std::filesystem;
    const fs::path inputPath(argv[1]);
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
    vector<pair<long long, Partition>> solutions;

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
        CKK solution(k, numbers);
        solutions.push_back(solution.solve());
    }

    const fs::path outputDir = "./output";
    fs::create_directories(outputDir);
    const fs::path outputPath = outputDir / (inputPath.stem().string() + "_output.txt");
    ofstream outputFile(outputPath);
    if (!outputFile.is_open()) {
        cerr << "Failed to create output file.\n";
        return 1;
    }
    outputFile << numberOfSets << ' ' << k << '\n';
    for (int i = 0; i < numberOfSets; ++i) {
        outputFile << '\n';
        writeSolution(outputFile, sets[i], solutions[i].first, solutions[i].second);
    }
    if (!outputFile) {
        cerr << "Error writing output file.\n";
        return 1;
    }
    cout << "Results written to " << outputPath.string() << '\n';
    return 0;
}
