#pragma once
#include <vector>
#include <string>
#include <functional>
#include <cmath>
#include <algorithm>

namespace algo {

// ── Bloom Filter ──────────────────────────────────────────────────────────────
class BloomFilter {
    std::vector<bool> bits_;
    int size_, k_;

    size_t hash(const std::string& s, int seed) const {
        size_t h = std::hash<std::string>{}(s);
        h ^= (size_t)seed * 0x9e3779b9;
        return h % size_;
    }
public:
    BloomFilter(int size, int numHashes)
        : bits_(size, false), size_(size), k_(numHashes) {}

    void add(const std::string& s) {
        for (int i = 0; i < k_; ++i) bits_[hash(s, i)] = true;
    }
    bool mightContain(const std::string& s) const {
        for (int i = 0; i < k_; ++i)
            if (!bits_[hash(s, i)]) return false;
        return true;
    }
    double falsePositiveRate(int n) const {
        return std::pow(1.0 - std::exp(-(double)k_ * n / size_), k_);
    }
};

// ── HyperLogLog (approximate cardinality) ────────────────────────────────────
class HyperLogLog {
    int m_;
    std::vector<uint8_t> regs_;

    static int clz(uint64_t x) {
        if (x == 0) return 64;
        int c = 0;
        while (!(x & (1ULL << 63))) { ++c; x <<= 1; }
        return c;
    }
public:
    explicit HyperLogLog(int b = 10) : m_(1 << b), regs_(1 << b, 0) {}

    void add(const std::string& s) {
        uint64_t h = std::hash<std::string>{}(s) * 0x9e3779b97f4a7c15ULL;
        int reg = h >> (64 - (int)std::log2(m_));
        uint8_t rank = (uint8_t)(clz(h << (int)std::log2(m_)) + 1);
        regs_[reg] = std::max(regs_[reg], rank);
    }
    double estimate() const {
        double sum = 0;
        for (auto v : regs_) sum += std::pow(2.0, -(double)v);
        double alpha = 0.7213 / (1.0 + 1.079 / m_);
        return alpha * m_ * m_ / sum;
    }
};

} // namespace algo
