#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <atomic>
#include <cstdlib>
#include <algorithm>
#include <random>

#define LIGHT
//#define MODERATE

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
    if (argc != 2) {
        std::cerr << "<usage> : ./cpu_workload <max_cores>\n";
        return 1;
    }

    int max_cores = std::atoi(argv[1]);
    int hardware_threads = std::thread::hardware_concurrency();
    max_cores = std::max(1, std::min(max_cores, hardware_threads));

    const int total_phases = 4;
    const double phase_duration = 5.0;

    std::vector<int> thread_plan;
    if(max_cores == 6){
#ifdef LIGHT
    thread_plan = {2, 1, 2, 1}; // ORIN NANO
#elif defined(MODERATE)
    thread_plan = {4, 3, 4, 3}; 
#else
    thread_plan = {1, 2, 3, 4}; 
#endif   
    }
    if(max_cores == 4){
 #ifdef LIGHT
    thread_plan = {2, 1, 2, 1}; // JET NANO, CORAL
#elif defined(MODERATE)
    thread_plan = {3, 2, 3, 2}; 
#else
    thread_plan = {1, 2, 3, 4}; 
#endif     
    }
    if(max_cores == 8){
 #ifdef LIGHT
    thread_plan = {3, 2, 3, 2}; // RUBIK, ODM2
#elif defined(MODERATE)
    thread_plan = {6, 4, 6, 4}; 
#else
    thread_plan = {1, 2, 3, 4}; 
#endif     
    }
    if(max_cores >8  || max_cores <4) {
    	std::cerr << "Unsupported core num\n";
	return 1;
    }
    for (int p = 0; p < total_phases; ++p) {
	int threads_to_run = thread_plan[p];
        std::atomic<bool> running_flag(true);
        std::vector<std::thread> threads;

        //std::cout << "[Phase " << (p + 1) << "] Running with "
        //          << threads_to_run << " thread(s) for " << phase_duration << " seconds...\n";

        for (int i = 0; i < threads_to_run; ++i) {
            threads.emplace_back(cpu_workload_function, std::ref(running_flag), i);
        }

        std::this_thread::sleep_for(std::chrono::duration<double>(phase_duration));
        running_flag = false;

        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }
    }

    //std::cout << "[CPU] === All workload phases complete ===\n";
    return 0;
}

