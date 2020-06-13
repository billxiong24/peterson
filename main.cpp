#include "peterson.h"
#include <thread>

static int num = 0;
const int TARGET = 4000000;
PetersonLock pl;

void func(short ind) {

    for (int i = 0; i < TARGET; i++) {
        pl.lock(ind);
        num++;
        pl.unlock(ind);
    }
}

int main(void) {
    std::thread t1(func, 0);
    std::thread t2(func, 1);
    t1.join();
    t2.join();

    std::cout << "num: " << num << std::endl;
    assert(num == 2*TARGET);
    return 0;
}
