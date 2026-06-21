// Reader-Writer Lock — allows concurrent readers OR exclusive writer access.
// Use case: read-heavy shared data (caches, config, routing tables) where
//           exclusive mutex locks would unnecessarily serialize readers.
// Complexity: O(1) acquire/release; readers never block each other.
// Policy:    Writer preference — new readers block when a writer is waiting,
//            preventing writer starvation.
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <chrono>

class RWLock {
    std::mutex              mu_;
    std::condition_variable cv_;
    int                     readers_{0};
    int                     writers_waiting_{0};
    bool                    writer_active_{false};

public:
    void lockRead() {
        std::unique_lock<std::mutex> lk(mu_);
        // Block if a writer is active or waiting (writer preference).
        cv_.wait(lk, [&]{ return !writer_active_ && writers_waiting_ == 0; });
        ++readers_;
    }

    void unlockRead() {
        std::unique_lock<std::mutex> lk(mu_);
        if (--readers_ == 0) cv_.notify_all();
    }

    void lockWrite() {
        std::unique_lock<std::mutex> lk(mu_);
        ++writers_waiting_;
        cv_.wait(lk, [&]{ return !writer_active_ && readers_ == 0; });
        --writers_waiting_;
        writer_active_ = true;
    }

    void unlockWrite() {
        std::unique_lock<std::mutex> lk(mu_);
        writer_active_ = false;
        cv_.notify_all();
    }
};

// RAII guards
struct ReadGuard {
    RWLock& rw;
    explicit ReadGuard(RWLock& r) : rw(r) { rw.lockRead(); }
    ~ReadGuard() { rw.unlockRead(); }
};

struct WriteGuard {
    RWLock& rw;
    explicit WriteGuard(RWLock& r) : rw(r) { rw.lockWrite(); }
    ~WriteGuard() { rw.unlockWrite(); }
};

int main() {
    RWLock rw;
    int shared_value = 0;
    std::atomic<int> read_ops{0}, write_ops{0};

    const int kReaders = 6, kWriters = 2, kOps = 50;
    std::vector<std::thread> threads;

    for (int i = 0; i < kReaders; ++i)
        threads.emplace_back([&]() {
            for (int j = 0; j < kOps; ++j) {
                ReadGuard g(rw);
                int v = shared_value; // read without modifying
                (void)v;
                ++read_ops;
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });

    for (int i = 0; i < kWriters; ++i)
        threads.emplace_back([&]() {
            for (int j = 0; j < kOps; ++j) {
                WriteGuard g(rw);
                ++shared_value;
                ++write_ops;
                std::this_thread::sleep_for(std::chrono::microseconds(50));
            }
        });

    for (auto& t : threads) t.join();

    std::cout << "Read ops:  " << read_ops.load() << "\n";
    std::cout << "Write ops: " << write_ops.load() << "\n";
    std::cout << "Final value (expected " << kWriters * kOps << "): " << shared_value << "\n";
    return 0;
}
