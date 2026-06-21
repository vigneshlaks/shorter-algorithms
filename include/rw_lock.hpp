#pragma once
#include <mutex>
#include <condition_variable>

// Reader-Writer Lock — concurrent reads, exclusive writes.
// Writer-preference: new readers block when a writer is waiting,
// preventing writer starvation on read-heavy workloads.
class RWLock {
    std::mutex              mu_;
    std::condition_variable cv_;
    int                     readers_{0};
    int                     writers_waiting_{0};
    bool                    writer_active_{false};

public:
    void lockRead() {
        std::unique_lock<std::mutex> lk(mu_);
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

struct ReadGuard {
    RWLock& rw;
    explicit ReadGuard(RWLock& r)  : rw(r) { rw.lockRead();  }
    ~ReadGuard()                           { rw.unlockRead(); }
};

struct WriteGuard {
    RWLock& rw;
    explicit WriteGuard(RWLock& r) : rw(r) { rw.lockWrite();  }
    ~WriteGuard()                          { rw.unlockWrite(); }
};
