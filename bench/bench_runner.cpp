#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <functional>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>

#include "sorting.hpp"
#include "graph.hpp"
#include "dynamic_programming.hpp"
#include "probabilistic.hpp"

using Clock = std::chrono::high_resolution_clock;

struct Result {
    std::string name;
    int n;
    double ms;
};

// Returns elapsed milliseconds for running fn() `reps` times, averaged.
static double timeMs(std::function<void()> fn, int reps = 3) {
    // warmup
    fn();
    auto start = Clock::now();
    for (int i = 0; i < reps; ++i) fn();
    auto end = Clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count() / reps;
}

static std::vector<int> randVec(int n, int seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(0, n * 10);
    std::vector<int> v(n);
    for (auto& x : v) x = dist(rng);
    return v;
}

static algo::Graph randGraph(int n, int edgesPerNode, int seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> nodeDist(0, n - 1);
    std::uniform_int_distribution<int> weightDist(1, 100);
    algo::Graph g(n);
    for (int u = 0; u < n; ++u)
        for (int k = 0; k < edgesPerNode; ++k) {
            int v = nodeDist(rng);
            int w = weightDist(rng);
            g[u].push_back({v, w});
        }
    return g;
}

static void writeJson(const std::vector<Result>& results, const std::string& path) {
    std::ofstream f(path);
    f << std::fixed << std::setprecision(4);
    f << "[\n";
    for (size_t i = 0; i < results.size(); ++i) {
        const auto& r = results[i];
        f << "  {\"algo\": \"" << r.name << "\", \"n\": " << r.n
          << ", \"ms\": " << r.ms << "}";
        if (i + 1 < results.size()) f << ",";
        f << "\n";
    }
    f << "]\n";
}

int main(int argc, char* argv[]) {
    std::string outPath = "bench_results.json";
    if (argc > 1) outPath = argv[1];

    const std::vector<int> sizes = {1000, 10000, 100000};
    std::vector<Result> results;

    std::cout << std::left << std::setw(30) << "Algorithm"
              << std::setw(12) << "N"
              << "Time (ms)\n"
              << std::string(55, '-') << "\n";

    auto record = [&](const std::string& name, int n, double ms) {
        results.push_back({name, n, ms});
        std::cout << std::left << std::setw(30) << name
                  << std::setw(12) << n
                  << std::fixed << std::setprecision(3) << ms << "\n";
    };

    // Sorting benchmarks
    for (int n : sizes) {
        auto v = randVec(n);

        record("merge_sort", n, timeMs([&]{ auto a = v; algo::mergeSort(a); }));
        record("quick_sort", n, timeMs([&]{ auto a = v; algo::quickSort(a); }));
        record("heap_sort",  n, timeMs([&]{ auto a = v; algo::heapSort(a); }));

        auto sorted = v;
        std::sort(sorted.begin(), sorted.end());
        record("binary_search", n, timeMs([&]{
            algo::binarySearch(sorted, sorted[n / 2]);
        }, 1000));
    }

    // Graph benchmarks
    for (int n : {500, 2000, 5000}) {
        auto g = randGraph(n, 8);
        record("dijkstra", n, timeMs([&]{ algo::dijkstra(n, 0, g); }));
    }

    // DP benchmarks
    for (int n : {100, 500, 1000}) {
        std::string a(n, 'a'), b(n, 'b');
        std::mt19937 rng(7);
        for (auto& c : a) c = 'a' + rng() % 4;
        for (auto& c : b) c = 'a' + rng() % 4;
        record("edit_distance", n, timeMs([&]{ algo::editDistance(a, b); }));
        record("lcs",           n, timeMs([&]{ algo::lcs(a, b); }));
    }

    // Probabilistic benchmarks
    for (int n : {10000, 100000}) {
        algo::BloomFilter bf(n * 10, 4);
        std::vector<std::string> items(n);
        for (int i = 0; i < n; ++i) items[i] = "item_" + std::to_string(i);

        record("bloom_filter_add", n, timeMs([&]{
            algo::BloomFilter tmp(n * 10, 4);
            for (const auto& s : items) tmp.add(s);
        }));

        for (const auto& s : items) bf.add(s);
        record("bloom_filter_query", n, timeMs([&]{
            for (const auto& s : items) bf.mightContain(s);
        }));
    }

    writeJson(results, outPath);
    std::cout << "\nResults written to " << outPath << "\n";
    return 0;
}
