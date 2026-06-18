// Skip List — a probabilistic alternative to balanced trees for ordered data.
// Use case: Redis sorted sets are implemented as skip lists; useful anywhere
//           you need fast ordered insert/search/range queries with simpler
//           code than a red-black or AVL tree.
// Complexity: O(log n) expected for search/insert/delete.
#include <iostream>
#include <vector>
#include <random>
#include <limits>

using namespace std;

const int MAX_LEVEL = 4;

struct Node {
    int value;
    vector<Node*> forward;
    Node(int v, int level) : value(v), forward(level + 1, nullptr) {}
};

class SkipList {
    Node* head;
    int level = 0;
    mt19937 rng;

    int randomLevel() {
        int lvl = 0;
        while (lvl < MAX_LEVEL && (rng() % 2)) ++lvl;
        return lvl;
    }

public:
    SkipList() : head(new Node(numeric_limits<int>::min(), MAX_LEVEL)), rng(7) {}

    void insert(int value) {
        vector<Node*> update(MAX_LEVEL + 1, head);
        Node* x = head;
        for (int i = level; i >= 0; --i) {
            while (x->forward[i] && x->forward[i]->value < value) x = x->forward[i];
            update[i] = x;
        }
        int newLevel = randomLevel();
        if (newLevel > level) {
            for (int i = level + 1; i <= newLevel; ++i) update[i] = head;
            level = newLevel;
        }
        Node* newNode = new Node(value, newLevel);
        for (int i = 0; i <= newLevel; ++i) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
    }

    bool search(int value) const {
        Node* x = head;
        for (int i = level; i >= 0; --i) {
            while (x->forward[i] && x->forward[i]->value < value) x = x->forward[i];
        }
        x = x->forward[0];
        return x && x->value == value;
    }

    void print() const {
        Node* x = head->forward[0];
        while (x) { cout << x->value << " "; x = x->forward[0]; }
        cout << "\n";
    }
};

int main() {
    SkipList list;
    for (int v : {3, 6, 7, 9, 12, 19, 17, 26, 21, 25}) list.insert(v);

    cout << "Skip list contents (sorted): ";
    list.print();

    cout << "Search 19: " << (list.search(19) ? "found" : "not found") << "\n";
    cout << "Search 100: " << (list.search(100) ? "found" : "not found") << "\n";
    return 0;
}
