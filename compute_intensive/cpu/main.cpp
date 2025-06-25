#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <atomic>
#include <cstdlib>
#include <algorithm>
#include <random>

void cpu_workload_function(std::atomic<bool>& running_flag, int thread_id, int work_level) {
    volatile double result = 0.0;
    int inner_loop_count = 2 * work_level;

    while (running_flag) {
        for (int i = 1; i < inner_loop_count; ++i) {
            result += i * 0.001;
            result += i / (i + 1.0);
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "<usage> : ./cpu_workload <duration_sec(float)> <max_threads> <workload_level>\n";
        return 1;
    }

    double total_duration = std::atof(argv[1]);
    int max_threads = std::atoi(argv[2]);
    int workload_level = std::atoi(argv[3]);

    int hardware_threads = std::thread::hardware_concurrency();
    max_threads = std::max(1, std::min(max_threads, hardware_threads));
    workload_level = std::max(1, std::min(workload_level, 10));

    if (total_duration <= 0 || max_threads <= 0) {
        std::cerr << "Error: duration and thread count must be positive.\n";
        return 1;
    }

    double interval_duration = total_duration / max_threads;

    std::vector<int> thread_order(max_threads);
    for (int i = 0; i < max_threads; ++i) {
        thread_order[i] = i + 1;
    }

    std::mt19937 g(0625);
    std::shuffle(thread_order.begin(), thread_order.end(), g);

    std::cout << "[CPU] === Initial sleep (5s) ===\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));

    for (int phase = 0; phase < max_threads; ++phase) {
        int thread_count = thread_order[phase];
        std::atomic<bool> running_flag(true);
        std::vector<std::thread> threads;

        std::cout << "[Phase " << (phase + 1) << "] Running with " << thread_count << " thread(s) for " 
                  << interval_duration << " seconds...\n";

        for (int i = 0; i < thread_count; ++i) {
            threads.emplace_back(cpu_workload_function, std::ref(running_flag), i, workload_level);
        }

        std::this_thread::sleep_for(std::chrono::duration<double>(interval_duration));
        running_flag = false;

        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }
    }

    std::cout << "[CPU] === All workload phases complete ===\n";
    return 0;
}

