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
    if (argc != 3) {
        std::cout << "<usage> : ./cpu_workload <workload_level> <max_cores>\n";
        return 1;
    }

    int workload_level = std::atoi(argv[1]);
    int max_cores = std::atoi(argv[2]);

    int hardware_threads = std::thread::hardware_concurrency();
    max_cores = std::max(1, std::min(max_cores, hardware_threads));
    workload_level = std::max(1, std::min(workload_level, 10));

    const int total_phases = 4;
    const double phase_duration = 1.5;
    const int phase_interval_ms = 1000;

    std::vector<int> phase_order = {0, 1, 2, 3};
    std::mt19937 rng(625);
    std::shuffle(phase_order.begin(), phase_order.end(), rng);

    std::cout << "[CPU] === Initial sleep (5s) ===\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));

    for (int p = 0; p < total_phases; ++p) {
        int phase = phase_order[p];
        int threads_to_run = (max_cores * (phase + 1)) / total_phases;
        std::atomic<bool> running_flag(true);
        std::vector<std::thread> threads;

        std::cout << "[Phase " << (phase + 1) << " / Order " << (p + 1) << "] Running with " 
                  << threads_to_run << " thread(s) for " << phase_duration << " seconds...\n";

        for (int i = 0; i < threads_to_run; ++i) {
            threads.emplace_back(cpu_workload_function, std::ref(running_flag), i, workload_level);
        }

        std::this_thread::sleep_for(std::chrono::duration<double>(phase_duration));
        running_flag = false;

        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }

        if (p < total_phases - 1) {
            std::cout << "[Phase " << (phase + 1) << "] Sleeping for " << phase_interval_ms << " ms between phases...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(phase_interval_ms));
        }
    }

    std::cout << "[CPU] === All workload phases complete ===\n";
    return 0;
}

