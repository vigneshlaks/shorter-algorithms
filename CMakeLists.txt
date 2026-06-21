cmake_minimum_required(VERSION 3.16)
project(ShortFormAlgorithms CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall")

include_directories(include)

# ── Demo binaries (one per algorithm) ────────────────────────────────────────
file(GLOB_RECURSE DEMO_SRCS
    "graph/*.cpp"
    "dynamic_programming/*.cpp"
    "strings/*.cpp"
    "sorting_searching/*.cpp"
    "probabilistic_data_structures/*.cpp"
    "distributed_systems/*.cpp"
    "similarity_search/*.cpp"
    "optimization/*.cpp"
    "database_internals/*.cpp"
    "concurrency/*.cpp"
)

foreach(SRC ${DEMO_SRCS})
    file(RELATIVE_PATH REL "${CMAKE_SOURCE_DIR}" "${SRC}")
    string(REPLACE "/" "__" TARGET_NAME "${REL}")
    string(REPLACE ".cpp" "" TARGET_NAME "${TARGET_NAME}")
    add_executable(demo__${TARGET_NAME} ${SRC})
    set_target_properties(demo__${TARGET_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/demos")
endforeach()

# ── Benchmark harness ─────────────────────────────────────────────────────────
add_executable(bench bench/bench_runner.cpp)
set_target_properties(bench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# ── Test runner ───────────────────────────────────────────────────────────────
add_executable(tests tests/test_runner.cpp)
set_target_properties(tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# ── Real graph benchmarks ─────────────────────────────────────────────────────
add_executable(real_graph_bench scripts/run_real_graph.cpp)
add_executable(osm_astar scripts/run_osm_astar.cpp)
add_executable(heuristic_experiment scripts/heuristic_experiment.cpp)
add_executable(divergence_analysis scripts/divergence_analysis.cpp)
add_executable(near_optimal_subgraph scripts/near_optimal_subgraph.cpp)
add_executable(betweenness scripts/betweenness.cpp)

# ── Route planning server ──────────────────────────────────────────────────────
add_executable(route_server server/main.cpp)
target_include_directories(route_server PRIVATE server)
set_target_properties(route_server PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
set_target_properties(real_graph_bench PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

# ── CLI ───────────────────────────────────────────────────────────────────────
add_executable(algo cli/main.cpp)
set_target_properties(algo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
