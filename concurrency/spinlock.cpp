// Spinlock — mutual exclusion via atomic busy-wait.
// Use case: protecting very short critical sections where the cost of
//           a context switch (mutex sleep/wake) exceeds the wait time.
//           Common in OS kernels and high-frequency trading systems.
// Complexity: O(1) lock/unlock; threads spin (burn CPU) while waiting.
// Warning:   Never hold a spinlock across a blocking call or sleep.
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

class Spinlock {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
public:
    void lock() {
        // test_and_set returns true if flag was already set (locked).
        // Spin until we are the one who set it from false -> true.
        while (flag_.test_and_set(std::memory_order_acquire))
            ; // busy-wait
    }
    void unlock() {
        flag_.clear(std::memory_order_release);
    }
};

// RAII guard, mirrors std::lock_guard
struct SpinGuard {
    Spinlock& sl;
    explicit SpinGuard(Spinlock& s) : sl(s) { sl.lock(); }
    ~SpinGuard() { sl.unlock(); }
};

int main() {
    Spinlock sl;
    int counter = 0;
    const int kThreads = 8, kIncrements = 10000;

    auto worker = [&]() {
        for (int i = 0; i < kIncrements; ++i) {
            SpinGuard g(sl);
            ++counter;
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < kThreads; ++i) threads.emplace_back(worker);
    for (auto& t : threads) t.join();

    std::cout << "Counter (expected " << kThreads * kIncrements << "): " << counter << "\n";
    return 0;
}
