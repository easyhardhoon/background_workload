#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <atomic>
#include <cstdlib>
#include <algorithm>

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
    int max_cores = std::thread::hardware_concurrency();
    int num_threads = 6;         
    int duration_sec = 10;       
    int workload_level = 10;   
    if (argc !=4) {
	    std::cout << "<usage> : ./cpu_workload <num_threads> <duration> <level>\n";
	    return 1;
    }
    if (argc >= 2) num_threads = std::atoi(argv[1]);
    if (argc >= 3) duration_sec = std::atoi(argv[2]);
    if (argc >= 4) workload_level = std::atoi(argv[3]);

    num_threads = std::max(1, std::min(num_threads, max_cores));
    workload_level = std::max(1, std::min(workload_level, 10));

    //std::cout << "Launching CPU workload with:\n"
    //          << " - Threads: " << num_threads << " (Max cores: " << max_cores << ")\n"
    //          << " - Duration: " << duration_sec << " sec\n"
    //          << " - Workload Level: " << workload_level << " (1=Low ~ 10=Max)\n";

    //std::cout << "=== Initial sleep (5s) ===\n";
    std::this_thread::sleep_for(std::chrono::seconds(5));
    
    std::atomic<bool> running_flag(true);
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(cpu_workload_function, std::ref(running_flag), i, workload_level);
    }

    std::this_thread::sleep_for(std::chrono::seconds(duration_sec));
    running_flag = false;

    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }

    //std::cout << "CPU workload complete.\n";
    return 0;
}

