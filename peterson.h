#ifndef PETERSON_H
#define PETERSON_H

#include <iostream>
#include <atomic>
#include <thread>

class PetersonLock {
    public:
        PetersonLock();
        ~PetersonLock() = default;

        void lock(int ind); 
        void unlock(int ind);

    private:
        std::atomic<int> flag[2];
        std::atomic<int> turn;
};

#endif
