#pragma once
#include <vector>
#include <algorithm>

namespace algo {

// ── Merge Sort ────────────────────────────────────────────────────────────────
namespace detail {
inline void merge(std::vector<int>& a, int l, int m, int r) {
    std::vector<int> left(a.begin() + l, a.begin() + m + 1);
    std::vector<int> right(a.begin() + m + 1, a.begin() + r + 1);
    size_t i = 0, j = 0; int k = l;
    while (i < left.size() && j < right.size())
        a[k++] = (left[i] <= right[j]) ? left[i++] : right[j++];
    while (i < left.size()) a[k++] = left[i++];
    while (j < right.size()) a[k++] = right[j++];
}
}

inline void mergeSort(std::vector<int>& a, int l, int r) {
    if (l >= r) return;
    int m = l + (r - l) / 2;
    mergeSort(a, l, m);
    mergeSort(a, m + 1, r);
    detail::merge(a, l, m, r);
}

inline void mergeSort(std::vector<int>& a) {
    if (!a.empty()) mergeSort(a, 0, (int)a.size() - 1);
}

// ── Quick Sort ────────────────────────────────────────────────────────────────
inline void quickSort(std::vector<int>& a, int l, int r) {
    if (l >= r) return;
    int pivot = a[r], i = l - 1;
    for (int j = l; j < r; ++j)
        if (a[j] <= pivot) std::swap(a[++i], a[j]);
    std::swap(a[++i], a[r]);
    quickSort(a, l, i - 1);
    quickSort(a, i + 1, r);
}

inline void quickSort(std::vector<int>& a) {
    if (!a.empty()) quickSort(a, 0, (int)a.size() - 1);
}

// ── Heap Sort ─────────────────────────────────────────────────────────────────
inline void heapify(std::vector<int>& a, int n, int i) {
    int largest = i, l = 2*i+1, r = 2*i+2;
    if (l < n && a[l] > a[largest]) largest = l;
    if (r < n && a[r] > a[largest]) largest = r;
    if (largest != i) { std::swap(a[i], a[largest]); heapify(a, n, largest); }
}

inline void heapSort(std::vector<int>& a) {
    int n = a.size();
    for (int i = n/2 - 1; i >= 0; --i) heapify(a, n, i);
    for (int i = n - 1; i > 0; --i) { std::swap(a[0], a[i]); heapify(a, i, 0); }
}

// ── Binary Search ─────────────────────────────────────────────────────────────
inline int binarySearch(const std::vector<int>& a, int target) {
    int l = 0, r = (int)a.size() - 1;
    while (l <= r) {
        int m = l + (r - l) / 2;
        if (a[m] == target) return m;
        else if (a[m] < target) l = m + 1;
        else r = m - 1;
    }
    return -1;
}

} // namespace algo
