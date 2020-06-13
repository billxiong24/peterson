#include "peterson.h"
#include <assert.h>

PetersonLock::PetersonLock() : flag(), turn(0) {}

void PetersonLock::lock(short ind) {
    assert(ind == 0 || ind == 1);
    short other = !ind;

    /**
     * Can be relaxed memory order, since reads from flag[] occur after turn.exchange(),
     * which will synchronize this write by the time it is read. The only requirement 
     * is that stores to flag[ind] * be atomic in order to avoid data races. 
     */
    flag[ind].store(1, std::memory_order_relaxed);

    /**
     * Need atomic exchange for acquire-release ordering, so each thread's exchange()
     * synchronizes with the other. For this reason, we cannot use turn.store().
     */
    turn.exchange(other, std::memory_order_acq_rel);
    /**
     * flag load synchronizes with flag store in unlock(). This is to ensure that the next thread
     * to acquire the lock sees all modifications that the previous thread made. This is guaranteed
     * by acquire/release memory ordering, which synchronizes all memory stores before memory_order_release store operation.
     *
     * Three scenarios to test for correctness: 
     * 1) If thread 0 acquires lock, thread 1 will not:
     *
     * Assuming thread(ind=0) acquires lock. Then flag[0]=1, turn=1.
     * This means thread 0 is this first to execute turn.exchange()
     * Since turn.exchange() is atomic, thread 1 can only executes turn.exchange()
     * once thread 0 has finished. Since turn.exchange() follows acquire/release semantics,
     * thread 1's turn.exchange() synchronizes with thread 0's turn.exchange(). Due to 
     * acquire/release semantics, this also means all of thread 0's memory writes are synchronized
     * from thread 1's perspective- thread 1 will see the latest write to flag[0].
     * Thus, after thread 1's call to turn.exchange(), the following is guaranteed to be true from thread 1's point of view: 
     *                                      turn == 0
     *                                      flag[0] = 1
     * 
     * Thus, thread 1 will block until thread 0 releases lock. 
     *
     *
     * 2) If thread 0 acquires then releases lock, thread 1 will acquire lock:
     * The while loop guarantees that eventually, thread 1 will see flag[0] == 0, and acquire the lock.
     *
     *
     * 3) If thread 0 does not acquire lock, thread 1 will acquire lock:
     *
     * Assume thread 0 fails to acquire the lock.
     * This means thread 0 sees flag[1] == 1 && turn == 1. Only thread 1 writes to flag[1].
     *
     * If turn == 1, there are 2 possible cases:
     *  - thread 0's write to turn happens-before thread 1's write, but thread 1's write is not yet visible to thread 0
     *  - thread 0 over-wrote thread 1's write to turn. (i.e. thread 1's write happens-before thread 0's
     *
     * In either case, the read from flag[1] is always synchronized due to acquire/release ordering of turn.exchange().
     * So thread 0 will always see flag[1] == 1. Similarly, thread 1 will always see flag[0] == 1. 
     *
     * The only case in which thread 0 fails to acquire the lock is in the second case - if thread 0 overrides thread 1's write. This is the assumption.
     * In this case, thread 1 will eventually see turn == 1 (since that was the last write to turn), exit the loop, and acquire the lock.
     *
     *
     * The most important thing is that either thread will always see the latest write to flag[0] or flag[1] when reading from flag.
     * This is because of acquire/release memory ordering constraint on turn.exchange(), which flushes cache/store buffers so that
     * any subsequent read of previously stored variables are synchronized. 
     *
     */
    while(flag[other].load(std::memory_order_acquire) == 1 && turn.load(std::memory_order_relaxed) == other) {
        std::this_thread::yield();
    }
}

void PetersonLock::unlock(short ind) {
    flag[ind].store(0, std::memory_order_release);
}
