#ifndef AIZO_PROJECT2_TIMER_H
#define AIZO_PROJECT2_TIMER_H

#include <chrono>

class Timer {
    std::chrono::high_resolution_clock::time_point startTime;

public:
    void start() {
        startTime = std::chrono::high_resolution_clock::now();
    }

    long long stopMicroseconds() const {
        auto endTime = std::chrono::high_resolution_clock::now();

        return std::chrono::duration_cast<std::chrono::microseconds>(
            endTime - startTime
        ).count();
    }
};


#endif //AIZO_PROJECT2_TIMER_H
