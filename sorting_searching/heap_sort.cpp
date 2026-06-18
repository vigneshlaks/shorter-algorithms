// Heapsort — in-place O(n log n) sort, no extra memory needed.
// Use case: memory-constrained environments where merge sort's extra buffer
//           is too costly, or where worst-case guarantees matter (unlike quicksort).
#include <iostream>
#include <vector>

using namespace std;

void heapify(vector<int>& a, int n, int i) {
    int largest = i, l = 2*i + 1, r = 2*i + 2;
    if (l < n && a[l] > a[largest]) largest = l;
    if (r < n && a[r] > a[largest]) largest = r;
    if (largest != i) {
        swap(a[i], a[largest]);
        heapify(a, n, largest);
    }
}

void heapSort(vector<int>& a) {
    int n = a.size();
    for (int i = n/2 - 1; i >= 0; --i) heapify(a, n, i); // build max-heap
    for (int i = n - 1; i > 0; --i) {
        swap(a[0], a[i]);     // move max to the end
        heapify(a, i, 0);     // re-heapify the reduced heap
    }
}

int main() {
    vector<int> a = {12, 11, 13, 5, 6, 7};
    cout << "Before: "; for (int x : a) cout << x << " "; cout << "\n";
    heapSort(a);
    cout << "After:  "; for (int x : a) cout << x << " "; cout << "\n";
    return 0;
}
