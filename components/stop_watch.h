#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <iostream>
#include <chrono>
#include <thread>

class Stopwatch {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

private:
    TimePoint start_time_;
    
    std::chrono::duration<double> elapsed_; 
    bool running_ = false;

    // Private constructor to prevent instantiation
    Stopwatch() {
        elapsed_ = std::chrono::duration<double>(0);
    }

public:
    // Static method to get the singleton instance
    static Stopwatch& getInstance() {
        static Stopwatch instance;
        return instance;
    }

    void start() {
        if (!running_) {
            start_time_ = Clock::now();
            running_ = true;
        }
    }

    void stop() {
        if (running_) {
            elapsed_ += Clock::now() - start_time_;
            running_ = false;
        }
    }

    double elapsedSeconds() const {
        std::chrono::duration<double> total_elapsed = elapsed_;
        if (running_) {
            total_elapsed += Clock::now() - start_time_;
        }
        return total_elapsed.count();
    }

    void reset() {
        elapsed_ = std::chrono::duration<double>(0);
        running_ = false;
    }

    // Delete the copy constructor and assignment operator
    Stopwatch(const Stopwatch&) = delete;
    Stopwatch& operator=(const Stopwatch&) = delete;
};

#endif