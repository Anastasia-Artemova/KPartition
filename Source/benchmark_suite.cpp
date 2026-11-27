#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <random>
#include <iomanip>
#include <cmath>
#include <climits>
#include <map>
#include <set>
#include <stdexcept>
#include "cli_helper.hpp"

using namespace std;


class Partition {
public:
    vector<long long> sums;
    vector<vector<int>> elements;

    explicit Partition(int k) : sums(k, 0), elements(k) {}

    void addElement(int el, int idx) {
        sums.at(idx) += el;
        elements.at(idx).push_back(el);
    }

    long long getCurrDifference() const {
        auto [minIt, maxIt] = minmax_element(sums.begin(), sums.end());
        return *maxIt - *minIt;
    }

    bool hasEmptySubset() const {
        return any_of(elements.begin(), elements.end(), [](const vector<int>& g) { return g.empty(); });
    }

    static vector<Partition> generatePermutations(const Partition& p1, const Partition& p2) {
        vector<Partition> result;
        int k = static_cast<int>(p2.sums.size());
        map<long long, vector<int>> groups;
        for (int i = 0; i < k; ++i) groups[p2.sums[i]].push_back(i);

        vector<long long> values;
        for (const auto& [sum, indices] : groups) {
            for (size_t j = 0; j < indices.size(); ++j) values.push_back(sum);
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

        stable_sort(result.begin(), result.end(), [](const Partition& a, const Partition& b) {
            return a.getCurrDifference() < b.getCurrDifference();
        });
        return result;
    }
};

class CKKSolver {
private:
    int k;
    vector<Partition> partitions;
    long long bestDiff = LLONG_MAX;
    chrono::steady_clock::time_point deadline;
    bool timedOut = false;

    void recursiveSolver(vector<Partition> current) {
        if (bestDiff == 0 || timedOut) return;
        if (chrono::steady_clock::now() > deadline) {
            timedOut = true;
            return;
        }
        if (current.size() == 1) {
            if (current[0].hasEmptySubset()) return;
            long long diff = current[0].getCurrDifference();
            if (diff < bestDiff) bestDiff = diff;
            return;
        }

        stable_sort(current.begin(), current.end(), [](const Partition& a, const Partition& b) {
            return a.getCurrDifference() > b.getCurrDifference();
        });

        auto perms = Partition::generatePermutations(current[0], current[1]);
        vector<Partition> remaining(current.begin() + 2, current.end());

        for (const auto& merged : perms) {
            vector<Partition> next = remaining;
            next.push_back(merged);
            recursiveSolver(move(next));
            if (bestDiff == 0 || timedOut) return;
        }
    }

public:
    CKKSolver(int k, vector<int> nums) : k(k) {
        sort(nums.rbegin(), nums.rend());
        for (int num : nums) {
            Partition p(k);
            p.addElement(num, 0);
            partitions.push_back(move(p));
        }
    }

    pair<long long, bool> solve(double timeoutSeconds) {
        deadline = chrono::steady_clock::now() + chrono::milliseconds(static_cast<long long>(timeoutSeconds * 1000));
        recursiveSolver(partitions);
        return {bestDiff, timedOut};
    }
};

class PolySolver {
private:
    int k;
    vector<int> nums;
    vector<vector<int>> result;
    vector<long long> sums;

    void findContainerWithLeastSum(int elToAdd) {
        int minIdx = 0;
        for (int i = 1; i < k; i++) {
            if (sums[i] < sums[minIdx]) minIdx = i;
        }
        result[minIdx].push_back(elToAdd);
        sums[minIdx] += elToAdd;
    }

    long long getDifference() const {
        auto [minIt, maxIt] = minmax_element(sums.begin(), sums.end());
        return *maxIt - *minIt;
    }

public:
    PolySolver(int k, vector<int> numbers)
        : k(k), nums(move(numbers)), result(k), sums(k, 0) {}

    pair<long long, long long> solve() {
        int n = static_cast<int>(nums.size());
        const long long MAX_ITERATIONS = 1LL * n * n;
        sort(nums.rbegin(), nums.rend());
        for (int el : nums) findContainerWithLeastSum(el);

        long long iteration = 0;
        while (iteration < MAX_ITERATIONS) {
            long long currDiff = getDifference();
            if (currDiff == 0) break;
            bool improved = false;
            for (int a = 0; a < k; a++) {
                for (int b = a + 1; b < k; b++) {
                    if (sums[a] == sums[b]) continue;
                    for (size_t i = 0; i < result[a].size(); i++) {
                        for (size_t j = 0; j < result[b].size(); j++) {
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
            if (!improved) break;
            ++iteration;
        }
        return {getDifference(), iteration};
    }
};


class EnergySolver {
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
            if (sums[i] < sums[minIdx]) minIdx = i;
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
        for (long long s : sums) energy += s * s;
        return energy;
    }

    void helper(vector<Subset>& allSubsets, Subset& currentSubset, int index, const vector<int>& container) const {
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
        for (int idx : indices) chosen[idx] = true;
        for (size_t i = 0; i < container.size(); i++) {
            if (chosen[i]) selected.push_back(container[i]);
            else remaining.push_back(container[i]);
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
    EnergySolver(int k, vector<int> numbers)
        : k(k), nums(move(numbers)), result(k), sums(k, 0) {}

    pair<long long, long long> solve() {
        sort(nums.rbegin(), nums.rend());
        for (int el : nums) findContainerWithLeastSum(el);

        vector<vector<int>> bestResult = result;
        vector<long long> bestSums = sums;
        long long bestDifference = getDifference();
        long long bestEnergy = calculateEnergy();
        long long iteration = 0;

        while (true) {
            if (bestDifference == 0) break;
            SwapCandidate best;
            long long bestDelta = 0;

            for (int a = 0; a < k; a++) {
                for (int b = a + 1; b < k; b++) {
                    if (sums[a] == sums[b]) continue;
                    vector<Subset> subsetsA = allSubsetsForOneContainer(a);
                    vector<Subset> subsetsB = allSubsetsForOneContainer(b);
                    int sizeA = static_cast<int>(result[a].size());
                    int sizeB = static_cast<int>(result[b].size());

                    for (const auto& subsetA : subsetsA) {
                        for (const auto& subsetB : subsetsB) {
                            int newSizeA = sizeA - static_cast<int>(subsetA.indices.size()) + static_cast<int>(subsetB.indices.size());
                            int newSizeB = sizeB - static_cast<int>(subsetB.indices.size()) + static_cast<int>(subsetA.indices.size());
                            if (newSizeA == 0 || newSizeB == 0) continue;
                            long long transfer = subsetA.sum - subsetB.sum;
                            if (transfer == 0) continue;
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

            if (best.groupA == -1) break;
            performSwap(best);
            iteration++;
            long long newEnergy = calculateEnergy();
            long long newDifference = getDifference();

            if (newDifference < bestDifference || (newDifference == bestDifference && newEnergy < bestEnergy)) {
                bestDifference = newDifference;
                bestEnergy = newEnergy;
                bestResult = result;
                bestSums = sums;
            }
        }
        result = move(bestResult);
        sums = move(bestSums);
        return {getDifference(), iteration};
    }
};


struct BenchStats {
    double mean_ms = 0;
    double std_ms = 0;
    double min_ms = 0;
    double max_ms = 0;
    double avg_diff = 0;
    double avg_iters = 0;
    int timeouts = 0;
    int total_trials = 0;
};

BenchStats computeStats(const vector<double>& times, const vector<long long>& diffs, const vector<long long>& iters, int timeouts) {
    BenchStats st;
    st.total_trials = static_cast<int>(times.size());
    st.timeouts = timeouts;
    if (times.empty()) return st;

    st.min_ms = *min_element(times.begin(), times.end());
    st.max_ms = *max_element(times.begin(), times.end());
    double sum = accumulate(times.begin(), times.end(), 0.0);
    st.mean_ms = sum / times.size();

    double var = 0;
    for (double t : times) var += (t - st.mean_ms) * (t - st.mean_ms);
    st.std_ms = times.size() > 1 ? sqrt(var / (times.size() - 1)) : 0.0;

    if (!diffs.empty()) {
        st.avg_diff = static_cast<double>(accumulate(diffs.begin(), diffs.end(), 0LL)) / diffs.size();
    }
    if (!iters.empty()) {
        st.avg_iters = static_cast<double>(accumulate(iters.begin(), iters.end(), 0LL)) / iters.size();
    }
    return st;
}


int main() {
    kpart_cli::ConsolePauseGuard pauseGuard;
    cout << fixed << setprecision(3);
    mt19937 rng(42); 

    cout << "========================================================\n";
    cout << "K-PARTITION COMPREHENSIVE TIME PERFORMANCE BENCHMARK\n";
    cout << "========================================================\n\n";
    cout << "### BENCHMARK 1: EXACT CKK ALGORITHM (5 trials per size, timeout=5s)\n";
    cout << "k,n,mean_ms,std_ms,min_ms,max_ms,avg_diff,timeouts\n";
    for (int k : {2, 3, 4}) {
        for (int n : {4, 6, 8, 10, 12, 14, 16, 18}) {
            if (n < k) continue;
            vector<double> times;
            vector<long long> diffs;
            vector<long long> dummy;
            int timeouts = 0;
            const int TRIALS = 5;

            for (int t = 0; t < TRIALS; t++) {
                vector<int> nums(n);
                for (int i = 0; i < n; i++) nums[i] = (rng() % 1000) + 1;

                CKKSolver solver(k, nums);
                auto t0 = chrono::high_resolution_clock::now();
                auto [diff, timedOut] = solver.solve(5.0); 
                auto t1 = chrono::high_resolution_clock::now();
                double el = chrono::duration<double, milli>(t1 - t0).count();

                if (timedOut) {
                    timeouts++;
                } else {
                    times.push_back(el);
                    diffs.push_back(diff);
                }
            }

            BenchStats st = computeStats(times, diffs, dummy, timeouts);
            cout << k << "," << n << "," << st.mean_ms << "," << st.std_ms << ","
                 << st.min_ms << "," << st.max_ms << "," << st.avg_diff << "," << st.timeouts << "\n";
            cout.flush();

            if (timeouts == TRIALS) break;
        }
    }

   
    cout << "\n### BENCHMARK 2: POLYNOMIAL APPROXIMATION (10 trials per size)\n";
    cout << "k,n,mean_ms,std_ms,min_ms,max_ms,avg_diff,avg_iters\n";
    for (int k : {2, 4, 8}) {
        for (int n : {10, 20, 50, 100, 150, 200}) {
            vector<double> times;
            vector<long long> diffs;
            vector<long long> iters;
            const int TRIALS = 10;

            for (int t = 0; t < TRIALS; t++) {
                vector<int> nums(n);
                for (int i = 0; i < n; i++) nums[i] = (rng() % 1000) + 1;

                PolySolver solver(k, nums);
                auto t0 = chrono::high_resolution_clock::now();
                auto [diff, it] = solver.solve();
                auto t1 = chrono::high_resolution_clock::now();
                double el = chrono::duration<double, milli>(t1 - t0).count();

                times.push_back(el);
                diffs.push_back(diff);
                iters.push_back(it);
            }

            BenchStats st = computeStats(times, diffs, iters, 0);
            cout << k << "," << n << "," << st.mean_ms << "," << st.std_ms << ","
                 << st.min_ms << "," << st.max_ms << "," << st.avg_diff << "," << st.avg_iters << "\n";
            cout.flush();
        }
    }

   
    cout << "\n### BENCHMARK 3: ENERGY-BASED HEURISTIC (10 trials per size)\n";
    cout << "k,n,density_n_k,mean_ms,std_ms,min_ms,max_ms,avg_diff,avg_iters,timeouts\n";
    for (int k : {4, 8}) {
        vector<int> n_values = (k == 4) ? vector<int>{8, 12, 16, 20, 24, 28, 32} 
                                        : vector<int>{16, 24, 32, 40, 48, 64, 80};
        for (int n : n_values) {
            vector<double> times;
            vector<long long> diffs;
            vector<long long> iters;
            int timeouts = 0;
            const int TRIALS = 10;

            for (int t = 0; t < TRIALS; t++) {
                vector<int> nums(n);
                for (int i = 0; i < n; i++) nums[i] = (rng() % 1000) + 1;

                EnergySolver solver(k, nums);
                auto t0 = chrono::high_resolution_clock::now();
                auto [diff, it] = solver.solve();
                auto t1 = chrono::high_resolution_clock::now();
                double el = chrono::duration<double, milli>(t1 - t0).count();

                times.push_back(el);
                diffs.push_back(diff);
                iters.push_back(it);
            }

            BenchStats st = computeStats(times, diffs, iters, timeouts);
            double density = static_cast<double>(n) / k;
            cout << k << "," << n << "," << density << "," << st.mean_ms << "," << st.std_ms << ","
                 << st.min_ms << "," << st.max_ms << "," << st.avg_diff << "," << st.avg_iters << "," << st.timeouts << "\n";
            cout.flush();
        }
    }

    return 0;
}
