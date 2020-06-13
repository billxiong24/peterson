#ifndef PETERSON_H
#define PETERSON_H

#include <iostream>
#include <atomic>
#include <thread>

class PetersonLock {
    public:
        PetersonLock();
        ~PetersonLock() = default;

        void lock(short ind); 
        void unlock(short ind);

    private:
        std::atomic<short> flag[2];
        std::atomic<short> turn;
};

#endif
