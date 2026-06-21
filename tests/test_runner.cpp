#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include <functional>

#include "sorting.hpp"
#include "graph.hpp"
#include "dynamic_programming.hpp"
#include "probabilistic.hpp"

// ── Minimal test harness ──────────────────────────────────────────────────────
static int passed = 0, failed = 0;

static void check(const std::string& name, bool ok) {
    if (ok) {
        std::cout << "  [PASS] " << name << "\n";
        ++passed;
    } else {
        std::cout << "  [FAIL] " << name << "\n";
        ++failed;
    }
}

// ── Sorting tests ─────────────────────────────────────────────────────────────
static void testSorting() {
    std::cout << "\nSorting & Searching\n";

    auto sorted = [](std::vector<int> v) {
        std::sort(v.begin(), v.end()); return v;
    };

    std::vector<int> base = {5, 3, 8, 1, 9, 2, 7, 4, 6};
    std::vector<int> expected = sorted(base);

    { auto a = base; algo::mergeSort(a); check("merge_sort: correctness", a == expected); }
    { auto a = base; algo::quickSort(a); check("quick_sort: correctness", a == expected); }
    { auto a = base; algo::heapSort(a);  check("heap_sort: correctness",  a == expected); }

    check("merge_sort: empty input", []{ std::vector<int> e; algo::mergeSort(e); return e.empty(); }());
    check("merge_sort: single element", []{ std::vector<int> e={1}; algo::mergeSort(e); return e==std::vector<int>{1}; }());

    std::vector<int> s = {1,3,5,7,9,11,13};
    check("binary_search: found",     algo::binarySearch(s, 7) == 3);
    check("binary_search: not found", algo::binarySearch(s, 4) == -1);
    check("binary_search: first",     algo::binarySearch(s, 1) == 0);
    check("binary_search: last",      algo::binarySearch(s, 13) == 6);
}

// ── Graph tests ───────────────────────────────────────────────────────────────
static void testGraph() {
    std::cout << "\nGraph Algorithms\n";

    // Triangle: 0-1 (w=4), 0-2 (w=1), 2-1 (w=2)
    algo::Graph g(3);
    g[0].push_back({1, 4}); g[0].push_back({2, 1});
    g[2].push_back({1, 2});

    auto dist = algo::dijkstra(3, 0, g);
    check("dijkstra: source dist=0",    dist[0] == 0);
    check("dijkstra: direct path",      dist[1] == 3); // 0->2->1 = 1+2=3, not 0->1=4
    check("dijkstra: shortest path",    dist[2] == 1);

    // Unreachable node
    algo::Graph g2(3);
    g2[0].push_back({1, 5});
    auto d2 = algo::dijkstra(3, 0, g2);
    check("dijkstra: unreachable node", d2[2] == std::numeric_limits<long long>::max());

    // BFS
    std::vector<std::vector<int>> adj(4);
    adj[0] = {1,2}; adj[1] = {3}; adj[2] = {3};
    auto bd = algo::bfs(4, 0, adj);
    check("bfs: source=0",   bd[0] == 0);
    check("bfs: depth 1",    bd[1] == 1 && bd[2] == 1);
    check("bfs: depth 2",    bd[3] == 2);
}

// ── DP tests ──────────────────────────────────────────────────────────────────
static void testDP() {
    std::cout << "\nDynamic Programming\n";

    check("edit_distance: identical",  algo::editDistance("abc", "abc") == 0);
    check("edit_distance: empty",      algo::editDistance("", "abc") == 3);
    check("edit_distance: kitten/sitting", algo::editDistance("kitten", "sitting") == 3);
    check("edit_distance: one deletion",   algo::editDistance("abc", "ac") == 1);

    check("lcs: common subseq",   algo::lcs("abcde", "ace") == 3);
    check("lcs: no common",       algo::lcs("abc", "xyz") == 0);
    check("lcs: identical",       algo::lcs("abc", "abc") == 3);

    check("knapsack: basic",
          algo::knapsack(50, {10,20,30}, {60,100,120}) == 220);
    check("knapsack: zero capacity", algo::knapsack(0, {1,2}, {3,4}) == 0);
    check("knapsack: single item fits",
          algo::knapsack(5, {5}, {10}) == 10);
    check("knapsack: single item too heavy",
          algo::knapsack(4, {5}, {10}) == 0);
}

// ── Probabilistic tests ───────────────────────────────────────────────────────
static void testProbabilistic() {
    std::cout << "\nProbabilistic Data Structures\n";

    algo::BloomFilter bf(1000, 4);
    bf.add("apple"); bf.add("banana"); bf.add("cherry");

    check("bloom_filter: inserted items present",
          bf.mightContain("apple") && bf.mightContain("banana") && bf.mightContain("cherry"));
    check("bloom_filter: obviously absent item",
          !bf.mightContain("xyzzy_definitely_not_inserted_ever"));

    // HyperLogLog: cardinality estimate within 10% for 10k unique items
    algo::HyperLogLog hll;
    for (int i = 0; i < 10000; ++i) hll.add("item_" + std::to_string(i));
    double est = hll.estimate();
    check("hyperloglog: ~10k cardinality within 10%", est > 9000 && est < 11000);
}

// ── Main ──────────────────────────────────────────────────────────────────────
int main() {
    std::cout << "=== Algorithm Test Suite ===\n";
    testSorting();
    testGraph();
    testDP();
    testProbabilistic();

    std::cout << "\n--- Results: " << passed << " passed, " << failed << " failed ---\n";
    return failed == 0 ? 0 : 1;
}
