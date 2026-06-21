// Bounded Buffer (Producer-Consumer) — classic synchronization problem.
// Use case: decoupling producers from consumers at different speeds;
//           used in I/O pipelines, thread pools, streaming systems.
// Mechanism: two semaphores track empty/full slots; mutex protects the buffer.
// Complexity: O(1) push/pop with blocking on empty/full conditions.
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <chrono>
#include <random>

template<typename T>
class BoundedBuffer {
    std::queue<T>           buf_;
    const size_t            capacity_;
    std::mutex              mu_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;

public:
    explicit BoundedBuffer(size_t cap) : capacity_(cap) {}

    void push(T item) {
        std::unique_lock<std::mutex> lk(mu_);
        not_full_.wait(lk, [&]{ return buf_.size() < capacity_; });
        buf_.push(std::move(item));
        not_empty_.notify_one();
    }

    T pop() {
        std::unique_lock<std::mutex> lk(mu_);
        not_empty_.wait(lk, [&]{ return !buf_.empty(); });
        T item = std::move(buf_.front());
        buf_.pop();
        not_full_.notify_one();
        return item;
    }

    size_t size() {
        std::lock_guard<std::mutex> lk(mu_);
        return buf_.size();
    }
};

int main() {
    BoundedBuffer<int> buf(8); // buffer holds at most 8 items
    const int kProducers = 3, kConsumers = 2, kItemsPerProducer = 20;
    std::atomic<int> consumed{0};
    std::atomic<long long> total{0};

    std::vector<std::thread> threads;

    for (int p = 0; p < kProducers; ++p)
        threads.emplace_back([&, p]() {
            std::mt19937 rng(p);
            for (int i = 0; i < kItemsPerProducer; ++i) {
                buf.push(p * 100 + i);
                std::this_thread::sleep_for(
                    std::chrono::microseconds(rng() % 500));
            }
        });

    const int kTotal = kProducers * kItemsPerProducer;

    for (int c = 0; c < kConsumers; ++c)
        threads.emplace_back([&]() {
            while (true) {
                int prev = consumed.load();
                if (prev >= kTotal) break;
                if (!consumed.compare_exchange_weak(prev, prev + 1)) continue;
                total += buf.pop();
            }
        });

    for (auto& t : threads) t.join();

    // Expected total: sum of all pushed values
    long long expected = 0;
    for (int p = 0; p < kProducers; ++p)
        for (int i = 0; i < kItemsPerProducer; ++i)
            expected += p * 100 + i;

    std::cout << "Items consumed: " << consumed.load() << " / " << kTotal << "\n";
    std::cout << "Sum (expected " << expected << "): " << total.load() << "\n";
    return 0;
}
