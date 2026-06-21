#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>

static void printHelp() {
    std::cout << R"(
Usage: algo <command> [options]

Commands:
  run <algo>         Run a specific algorithm demo
  bench [output]     Run all benchmarks (writes JSON to output, default: bench_results.json)
  test               Run the full test suite
  list               List all available algorithms
  help               Show this help message

Algorithms (for `run`):
  Sorting/Searching:  merge_sort, quick_sort, heap_sort, binary_search
  Graph:              dijkstra, bellman_ford, bfs, floyd_warshall, kruskal,
                      topological_sort, tarjan_scc, union_find, pagerank, ford_fulkerson
  Dynamic Prog:       edit_distance, lcs, knapsack
  Strings:            trie, kmp, rabin_karp
  Probabilistic:      bloom_filter, hyperloglog, count_min_sketch, cuckoo_filter
  Distributed:        raft, consistent_hashing, crdt, vector_clocks, gossip
  Similarity:         lsh_minhash
  Optimization:       simulated_annealing, branch_and_bound

Examples:
  algo run dijkstra
  algo bench results/my_run.json
  algo test
  algo list
)";
}

// Maps logical algorithm names to compiled demo binary paths (relative to CLI binary).
static std::string demoBinary(const std::string& name) {
    static const std::vector<std::pair<std::string,std::string>> table = {
        {"merge_sort",          "demos/demo__sorting_searching__merge_sort"},
        {"quick_sort",          "demos/demo__sorting_searching__quick_sort"},
        {"heap_sort",           "demos/demo__sorting_searching__heap_sort"},
        {"binary_search",       "demos/demo__sorting_searching__binary_search"},
        {"dijkstra",            "demos/demo__graph__dijkstra"},
        {"bellman_ford",        "demos/demo__graph__bellman_ford"},
        {"bfs",                 "demos/demo__graph__bfs_shortest_path"},
        {"floyd_warshall",      "demos/demo__graph__floyd_warshall"},
        {"kruskal",             "demos/demo__graph__kruskal_mst"},
        {"topological_sort",    "demos/demo__graph__topological_sort"},
        {"tarjan_scc",          "demos/demo__graph__tarjan_scc"},
        {"union_find",          "demos/demo__graph__union_find"},
        {"pagerank",            "demos/demo__graph__pagerank"},
        {"ford_fulkerson",      "demos/demo__graph__ford_fulkerson_maxflow"},
        {"edit_distance",       "demos/demo__dynamic_programming__edit_distance"},
        {"lcs",                 "demos/demo__dynamic_programming__longest_common_subsequence"},
        {"knapsack",            "demos/demo__dynamic_programming__knapsack_01"},
        {"trie",                "demos/demo__strings__trie"},
        {"kmp",                 "demos/demo__strings__kmp_search"},
        {"rabin_karp",          "demos/demo__strings__rabin_karp"},
        {"bloom_filter",        "demos/demo__probabilistic_data_structures__bloom_filter"},
        {"hyperloglog",         "demos/demo__probabilistic_data_structures__hyperloglog"},
        {"count_min_sketch",    "demos/demo__probabilistic_data_structures__count_min_sketch"},
        {"cuckoo_filter",       "demos/demo__probabilistic_data_structures__cuckoo_filter"},
        {"raft",                "demos/demo__distributed_systems__raft_leader_election"},
        {"consistent_hashing",  "demos/demo__distributed_systems__consistent_hashing"},
        {"crdt",                "demos/demo__distributed_systems__crdt_counter"},
        {"vector_clocks",       "demos/demo__distributed_systems__vector_clocks"},
        {"gossip",              "demos/demo__distributed_systems__gossip_protocol"},
        {"lsh_minhash",         "demos/demo__similarity_search__lsh_minhash"},
        {"simulated_annealing", "demos/demo__optimization__simulated_annealing_tsp"},
        {"branch_and_bound",    "demos/demo__optimization__branch_and_bound_tsp"},
    };
    for (const auto& [key, path] : table)
        if (key == name) return path;
    return "";
}

int main(int argc, char* argv[]) {
    if (argc < 2) { printHelp(); return 0; }

    std::string cmd = argv[1];

    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        printHelp();
        return 0;
    }

    if (cmd == "list") {
        std::cout << "Available algorithms:\n";
        for (const auto& name : {
            "merge_sort","quick_sort","heap_sort","binary_search",
            "dijkstra","bellman_ford","bfs","floyd_warshall","kruskal",
            "topological_sort","tarjan_scc","union_find","pagerank","ford_fulkerson",
            "edit_distance","lcs","knapsack",
            "trie","kmp","rabin_karp",
            "bloom_filter","hyperloglog","count_min_sketch","cuckoo_filter",
            "raft","consistent_hashing","crdt","vector_clocks","gossip",
            "lsh_minhash","simulated_annealing","branch_and_bound"
        }) std::cout << "  " << name << "\n";
        return 0;
    }

    if (cmd == "run") {
        if (argc < 3) { std::cerr << "Usage: algo run <algorithm>\n"; return 1; }
        std::string algo = argv[2];
        std::string bin = demoBinary(algo);
        if (bin.empty()) {
            std::cerr << "Unknown algorithm: " << algo << "\nRun `algo list` to see options.\n";
            return 1;
        }
        return std::system(bin.c_str());
    }

    if (cmd == "bench") {
        std::string out = (argc > 2) ? argv[2] : "bench_results.json";
        return std::system(("./bench " + out).c_str());
    }

    if (cmd == "test") {
        return std::system("./tests");
    }

    std::cerr << "Unknown command: " << cmd << "\nRun `algo help` for usage.\n";
    return 1;
}
