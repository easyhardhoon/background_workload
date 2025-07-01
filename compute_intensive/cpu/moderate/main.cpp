#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <atomic>
#include <cstdlib>
#include <algorithm>
#include <random>

#define MODERATE
// #define OVERLOADED

void cpu_workload_function(std::atomic<bool>& running_flag, int thread_id) {
    volatile double result = 0.0;
    const int work_level = 10;
    int inner_loop_count = 2 * work_level;

    while (running_flag) {
        for (int i = 1; i < inner_loop_count; ++i) {
            result += i * 0.001;
            result += i / (i + 1.0);
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "<usage> : ./cpu_workload <max_cores> <duration_sec>\n";
        return 1;
    }

    int max_cores = std::atoi(argv[1]);
    double duration_sec = std::atof(argv[2]);

    int hardware_threads = std::thread::hardware_concurrency();
    max_cores = std::max(1, std::min(max_cores, hardware_threads));

    int thread_ratio = 1; // default: 25% (1/4)

#ifdef MODERATE
    thread_ratio = 1; // 25%
#elif defined(OVERLOADED)
    thread_ratio = 3; // 75%
#endif

    int threads_to_run = static_cast<int>(std::ceil((max_cores * thread_ratio) / 4.0));
    std::atomic<bool> running_flag(true);
    std::vector<std::thread> threads;

    std::cout << "[CPU] Running with " << threads_to_run
              << " thread(s) for " << duration_sec << " seconds...\n";

    for (int i = 0; i < threads_to_run; ++i) {
        threads.emplace_back(cpu_workload_function, std::ref(running_flag), i);
    }

    std::this_thread::sleep_for(std::chrono::duration<double>(duration_sec));
    running_flag = false;

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    // std::cout << "[CPU] === Workload complete ===\n";
    return 0;
}

