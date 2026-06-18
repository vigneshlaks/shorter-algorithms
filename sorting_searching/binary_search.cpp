// Binary Search — O(log n) lookup in sorted data.
// Use case: database index lookups, "find first/last occurrence" problems,
//           searching any monotonic function for a threshold (binary search on answer).
#include <iostream>
#include <vector>

using namespace std;

// Returns index of target, or -1 if not found.
int binarySearch(const vector<int>& a, int target) {
    int lo = 0, hi = (int)a.size() - 1;
    while (lo <= hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] == target) return mid;
        if (a[mid] < target) lo = mid + 1;
        else hi = mid - 1;
    }
    return -1;
}

// Finds the first index where a[index] >= target (lower bound).
int lowerBound(const vector<int>& a, int target) {
    int lo = 0, hi = (int)a.size();
    while (lo < hi) {
        int mid = lo + (hi - lo) / 2;
        if (a[mid] < target) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

int main() {
    vector<int> a = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    int target = 23;
    cout << "Searching for " << target << " in sorted array.\n";
    int idx = binarySearch(a, target);
    cout << "Found at index: " << idx << "\n";
    cout << "Lower bound for 20: index " << lowerBound(a, 20) << " (value " << a[lowerBound(a, 20)] << ")\n";
    return 0;
}
