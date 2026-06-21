# Concurrency Algorithms

Low-level synchronization primitives and lock-free data structures.

| File | Algorithm | Key Idea |
|---|---|---|
| `spinlock.cpp` | Spinlock | Busy-wait via `atomic_flag::test_and_set`; no OS involvement |
| `lockfree_stack.cpp` | Lock-Free Stack (Treiber) | CAS loop + version tag to defeat ABA problem |
| `lockfree_queue.cpp` | Lock-Free Queue (Michael-Scott) | Separate head/tail pointers; used in Java's `ConcurrentLinkedQueue` |
| `rw_lock.cpp` | Reader-Writer Lock | Many concurrent readers OR one exclusive writer; writer preference |
| `bounded_buffer.cpp` | Bounded Buffer | Producer-consumer via condition variables; blocks on full/empty |

## When to use what

- **Spinlock** — protecting a counter or pointer swap (sub-microsecond critical section). Bad if the lock is ever held while doing I/O or sleeping.
- **Lock-Free Stack/Queue** — when lock contention is the bottleneck and you need wait-free progress guarantees. Harder to reason about than mutex-based structures.
- **Reader-Writer Lock** — read-heavy shared state (config, routing tables, caches). Useless if writes are as frequent as reads.
- **Bounded Buffer** — decoupling producers and consumers running at different rates; the capacity bound provides backpressure.

## Concepts to understand

- **Memory ordering** (`acquire`/`release`/`seq_cst`) — controls which memory operations are visible across threads after a synchronization point.
- **ABA problem** — a CAS succeeds even though a pointer was changed from A→B→A between the load and the CAS. Solved with version/tag counters.
- **False sharing** — two threads writing different variables that land on the same cache line cause cache-line ping-pong. Fixed with `alignas(64)` padding.
- **Spurious wakeups** — condition variable `wait` can unblock without a `notify`. Always re-check the condition in a loop (use the predicate overload).
