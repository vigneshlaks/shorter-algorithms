#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <cmath>
#include "../include/rw_lock.hpp"

// ── Bloom filter for fast "definitely not cached" check ───────────────────────
// Prevents hitting the exact cache (unordered_map) for queries we've never seen.
// False positives cause unnecessary exact-cache lookups (cheap).
// False negatives are impossible — we never skip a cached result.
class RouteBloom {
    std::vector<bool> bits_;
    int size_, k_;

    size_t hashKey(long long key, int seed) const {
        size_t h = std::hash<long long>{}(key);
        h ^= (size_t)seed * 0x9e3779b97f4a7c15ULL;
        return h % size_;
    }

    long long encode(int start, int goal) const {
        return ((long long)start << 20) ^ goal;
    }

public:
    RouteBloom(int size = 1 << 20, int k = 5)
        : bits_(size, false), size_(size), k_(k) {}

    void add(int start, int goal) {
        long long key = encode(start, goal);
        for (int i = 0; i < k_; ++i) bits_[hashKey(key, i)] = true;
    }

    bool mightContain(int start, int goal) const {
        long long key = encode(start, goal);
        for (int i = 0; i < k_; ++i)
            if (!bits_[hashKey(key, i)]) return false;
        return true;
    }

    double expectedFPR(int n) const {
        return std::pow(1.0 - std::exp(-(double)k_ * n / size_), k_);
    }
};

// ── Exact cache: stores computed route results ────────────────────────────────
struct CachedRoute {
    double cost;
    double alphaUsed;
    int    pathLen;
};

class RouteCache {
    std::unordered_map<long long, CachedRoute> store_;
    mutable RWLock rw_;
    int hits_ = 0, misses_ = 0;

    long long key(int s, int g) const { return ((long long)s << 20) ^ g; }

public:
    bool get(int s, int g, CachedRoute& out) const {
        ReadGuard lk(rw_);
        auto it = store_.find(key(s, g));
        if (it == store_.end()) { ++const_cast<int&>(misses_); return false; }
        out = it->second;
        ++const_cast<int&>(hits_);
        return true;
    }

    void put(int s, int g, CachedRoute r) {
        WriteGuard lk(rw_);
        store_[key(s, g)] = r;
    }

    int hits()   const { return hits_; }
    int misses() const { return misses_; }
    int size()   const { return store_.size(); }
};
