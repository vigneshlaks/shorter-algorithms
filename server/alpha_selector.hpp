#pragma once
#include "graph.hpp"
#include <vector>
#include <mutex>
#include <cmath>
#include <algorithm>

// Divides the city into a lat/lon grid. Each cell tracks observed heuristic
// quality — the ratio of A* path cost to optimal cost — and uses that to
// select an α that stays within a user-specified suboptimality bound.
//
// Key insight from the experiment: optimality breaks sharply above α=1.05
// on SF's road network, but mean cost ratio only degrades to ~1.06 even at
// α=4. The selector exploits this: it tries α=1.3 (6x faster than Dijkstra)
// and falls back to α=1.0 when a region's observed suboptimality exceeds budget.
class AlphaSelector {
    struct Cell {
        double sumRatio = 0.0;
        int    count    = 0;
        double alpha    = 1.0; // current selected α for this cell

        double meanRatio() const {
            return count > 0 ? sumRatio / count : 1.0;
        }
    };

    int gridW_, gridH_;
    double latMin_, latMax_, lonMin_, lonMax_;
    std::vector<Cell> cells_;
    mutable std::mutex mu_;
    double budget_; // max acceptable mean suboptimality (e.g. 0.02 = 2%)

    int cellIdx(double lat, double lon) const {
        int r = std::min((int)((lat - latMin_) / (latMax_ - latMin_) * gridH_), gridH_-1);
        int c = std::min((int)((lon - lonMin_) / (lonMax_ - lonMin_) * gridW_), gridW_-1);
        return std::max(0, r) * gridW_ + std::max(0, c);
    }

public:
    AlphaSelector(double latMin, double latMax, double lonMin, double lonMax,
                  int gridW = 10, int gridH = 10, double budget = 0.02)
        : gridW_(gridW), gridH_(gridH),
          latMin_(latMin), latMax_(latMax), lonMin_(lonMin), lonMax_(lonMax),
          cells_(gridW * gridH),
          budget_(budget) {}

    // Select α for a query originating at (lat, lon).
    // Starts aggressive (α=1.3), backs off if observed quality is poor.
    double selectAlpha(double lat, double lon) const {
        std::lock_guard<std::mutex> lk(mu_);
        const Cell& c = cells_[cellIdx(lat, lon)];
        return c.alpha;
    }

    // Update quality observation after a query.
    // costRatio = astar_cost / dijkstra_cost (1.0 = optimal).
    void observe(double lat, double lon, double costRatio) {
        std::lock_guard<std::mutex> lk(mu_);
        Cell& c = cells_[cellIdx(lat, lon)];
        c.sumRatio += costRatio;
        ++c.count;

        double mean = c.meanRatio();
        double subopt = mean - 1.0;

        // Adaptive α: if quality is within budget, try being more aggressive.
        // If quality exceeds budget, back off toward α=1.
        if (subopt < budget_ * 0.5 && c.alpha < 1.5)
            c.alpha = std::min(c.alpha + 0.05, 1.5);
        else if (subopt > budget_)
            c.alpha = std::max(c.alpha - 0.1, 1.0);
    }

    void printStats() const {
        std::lock_guard<std::mutex> lk(mu_);
        double sumAlpha = 0; int observed = 0;
        for (const auto& c : cells_) {
            if (c.count > 0) { sumAlpha += c.alpha; ++observed; }
        }
        std::cout << "AlphaSelector: " << observed << "/" << cells_.size()
                  << " cells observed, mean α="
                  << (observed > 0 ? sumAlpha/observed : 1.0) << "\n";
    }
};
