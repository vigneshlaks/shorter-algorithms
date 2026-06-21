// Lock-Free Stack (Treiber Stack) — push/pop without any mutex.
// Use case: high-throughput concurrent stacks where lock contention is a
//           bottleneck, e.g. memory allocator free-lists, work-stealing queues.
// Complexity: O(1) amortized push/pop; retries on CAS failure under contention.
// Hazard:    Naive implementation suffers ABA problem — addressed here by
//            tagging pointers with a version counter.
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <optional>

template<typename T>
class LockFreeStack {
    struct Node {
        T value;
        Node* next;
        explicit Node(T v) : value(std::move(v)), next(nullptr) {}
    };

    // Tagged pointer: packs a version counter alongside the raw pointer to
    // defeat the ABA problem (thread A pops, thread B pops+pushes same node,
    // thread A's CAS would wrongly succeed without the tag).
    struct TaggedPtr {
        Node* ptr;
        uintptr_t tag;
        bool operator==(const TaggedPtr& o) const {
            return ptr == o.ptr && tag == o.tag;
        }
    };

    std::atomic<TaggedPtr> head_{{nullptr, 0}};

public:
    void push(T value) {
        Node* node = new Node(std::move(value));
        TaggedPtr old_head = head_.load(std::memory_order_relaxed);
        TaggedPtr new_head;
        do {
            node->next = old_head.ptr;
            new_head = {node, old_head.tag + 1};
        } while (!head_.compare_exchange_weak(old_head, new_head,
                                               std::memory_order_release,
                                               std::memory_order_relaxed));
    }

    std::optional<T> pop() {
        TaggedPtr old_head = head_.load(std::memory_order_acquire);
        TaggedPtr new_head;
        do {
            if (!old_head.ptr) return std::nullopt;
            new_head = {old_head.ptr->next, old_head.tag + 1};
        } while (!head_.compare_exchange_weak(old_head, new_head,
                                               std::memory_order_acquire,
                                               std::memory_order_relaxed));
        T val = std::move(old_head.ptr->value);
        delete old_head.ptr;
        return val;
    }

    ~LockFreeStack() {
        while (pop()) {}
    }
};

int main() {
    LockFreeStack<int> stack;
    const int kThreads = 4, kOps = 1000;
    std::atomic<int> sum{0};

    // Producers push 0..kOps-1, consumers pop and accumulate.
    std::vector<std::thread> producers, consumers;

    for (int t = 0; t < kThreads; ++t)
        producers.emplace_back([&, t]() {
            for (int i = 0; i < kOps; ++i)
                stack.push(t * kOps + i);
        });

    for (auto& p : producers) p.join();

    for (int t = 0; t < kThreads; ++t)
        consumers.emplace_back([&]() {
            while (auto v = stack.pop()) sum += *v;
        });

    for (auto& c : consumers) c.join();

    // Expected sum: sum of 0..kThreads*kOps-1
    int expected = kThreads * kOps * (kThreads * kOps - 1) / 2;
    std::cout << "Sum (expected " << expected << "): " << sum.load() << "\n";
    return 0;
}
