// Quicksort — fast average-case in-place sort; default in many standard libraries.
// Use case: general-purpose sorting where average performance matters more
//           than worst-case guarantees.
#include <iostream>
#include <vector>

using namespace std;

int partition(vector<int>& a, int low, int high) {
    int pivot = a[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (a[j] < pivot) swap(a[++i], a[j]);
    }
    swap(a[i + 1], a[high]);
    return i + 1;
}

void quickSort(vector<int>& a, int low, int high) {
    if (low >= high) return;
    int p = partition(a, low, high);
    quickSort(a, low, p - 1);
    quickSort(a, p + 1, high);
}

int main() {
    vector<int> a = {10, 7, 8, 9, 1, 5};
    cout << "Before: "; for (int x : a) cout << x << " "; cout << "\n";
    quickSort(a, 0, a.size() - 1);
    cout << "After:  "; for (int x : a) cout << x << " "; cout << "\n";
    return 0;
}
