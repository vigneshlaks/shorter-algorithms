// Merge Sort — stable, predictable O(n log n) sort.
// Use case: anywhere stability matters (sorting records by multiple keys
//           while preserving original order of ties); external sorting of huge files.
#include <iostream>
#include <vector>

using namespace std;

void merge(vector<int>& a, int l, int m, int r) {
    vector<int> left(a.begin() + l, a.begin() + m + 1);
    vector<int> right(a.begin() + m + 1, a.begin() + r + 1);
    size_t i = 0, j = 0; int k = l;
    while (i < left.size() && j < right.size())
        a[k++] = (left[i] <= right[j]) ? left[i++] : right[j++];
    while (i < left.size()) a[k++] = left[i++];
    while (j < right.size()) a[k++] = right[j++];
}

void mergeSort(vector<int>& a, int l, int r) {
    if (l >= r) return;
    int m = l + (r - l) / 2;
    mergeSort(a, l, m);
    mergeSort(a, m + 1, r);
    merge(a, l, m, r);
}

int main() {
    vector<int> a = {38, 27, 43, 3, 9, 82, 10};
    cout << "Before: "; for (int x : a) cout << x << " "; cout << "\n";
    mergeSort(a, 0, a.size() - 1);
    cout << "After:  "; for (int x : a) cout << x << " "; cout << "\n";
    return 0;
}
