// Lock-Free Queue (Michael-Scott Queue, 1996) — the algorithm behind
// Java's ConcurrentLinkedQueue and many production systems.
// Use case: multi-producer multi-consumer pipelines without lock overhead,
//           e.g. thread pool task queues, async I/O event dispatch.
// Complexity: O(1) amortized enqueue/dequeue with CAS retries under contention.
// Key insight: separate head (dequeue) and tail (enqueue) pointers so
//              producers and consumers rarely contend with each other.
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <optional>

template<typename T>
class LockFreeQueue {
    struct Node {
        T                  value{};
        std::atomic<Node*> next{nullptr};
        explicit Node(T v = T{}) : value(std::move(v)) {}
    };

    // Separate cache lines to avoid false sharing between enqueue/dequeue.
    alignas(64) std::atomic<Node*> head_; // dummy sentinel node
    alignas(64) std::atomic<Node*> tail_;

public:
    LockFreeQueue() {
        Node* sentinel = new Node();
        head_.store(sentinel, std::memory_order_relaxed);
        tail_.store(sentinel, std::memory_order_relaxed);
    }

    void enqueue(T value) {
        Node* node = new Node(std::move(value));
        Node* prev_tail;
        // Swing tail to new node; the previous tail's next points to new node.
        while (true) {
            prev_tail = tail_.load(std::memory_order_acquire);
            Node* next = prev_tail->next.load(std::memory_order_acquire);
            if (prev_tail != tail_.load(std::memory_order_relaxed)) continue;
            if (next == nullptr) {
                if (prev_tail->next.compare_exchange_weak(next, node,
                        std::memory_order_release, std::memory_order_relaxed)) {
                    // Best-effort tail advance (another thread may do it too).
                    tail_.compare_exchange_weak(prev_tail, node,
                        std::memory_order_release, std::memory_order_relaxed);
                    return;
                }
            } else {
                // Tail is behind — help advance it.
                tail_.compare_exchange_weak(prev_tail, next,
                    std::memory_order_release, std::memory_order_relaxed);
            }
        }
    }

    std::optional<T> dequeue() {
        while (true) {
            Node* head = head_.load(std::memory_order_acquire);
            Node* tail = tail_.load(std::memory_order_acquire);
            Node* next = head->next.load(std::memory_order_acquire);

            if (head != head_.load(std::memory_order_relaxed)) continue;

            if (head == tail) {
                if (next == nullptr) return std::nullopt; // empty
                // Tail is behind — help advance it.
                tail_.compare_exchange_weak(tail, next,
                    std::memory_order_release, std::memory_order_relaxed);
                continue;
            }

            // Read value before CAS — once CAS succeeds another thread may free next.
            T value = next->value;
            if (head_.compare_exchange_weak(head, next,
                    std::memory_order_release, std::memory_order_relaxed)) {
                delete head; // old sentinel
                return value;
            }
        }
    }

    ~LockFreeQueue() {
        while (dequeue()) {}
        delete head_.load();
    }
};

int main() {
    LockFreeQueue<int> queue;
    const int kProducers = 4, kConsumers = 4, kOps = 2500;
    std::atomic<long long> total{0};
    std::atomic<int> produced{0}, consumed{0};
    const int kTotal = kProducers * kOps;

    std::vector<std::thread> producers, consumers;

    for (int t = 0; t < kProducers; ++t)
        producers.emplace_back([&]() {
            for (int i = 0; i < kOps; ++i) {
                queue.enqueue(1);
                ++produced;
            }
        });

    for (auto& p : producers) p.join();

    // Push one sentinel (-1) per consumer after all real items.
    for (int c = 0; c < kConsumers; ++c)
        queue.enqueue(-1);

    for (int t = 0; t < kConsumers; ++t)
        consumers.emplace_back([&]() {
            while (true) {
                auto v = queue.dequeue();
                if (!v) continue;          // queue momentarily empty, spin
                if (*v == -1) return;      // sentinel: done
                total += *v;
                ++consumed;
            }
        });

    for (auto& c : consumers) c.join();

    std::cout << "Items produced: " << produced.load() << "\n";
    std::cout << "Items consumed: " << consumed.load() << "\n";
    std::cout << "Sum (expected " << kTotal << "): " << total.load() << "\n";
    return 0;
}
