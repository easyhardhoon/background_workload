#include <unistd.h>
#include <iostream>
#include <cstdlib>      // for atoi
#include "egl_util.h"

void run_gpu_compute_workload(float base_level);

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cout << "<usage> : ./gl_workload <level>\n";
        return 1;
    }

    int level = std::atoi(argv[1]);
    if (level <= 0) {
        std::cerr << "Error: level must be a positive integer.\n";
        return 1;
    }

    //std::cout << "[GPU] === Initial sleep (5s) ===\n";
    //sleep(5);

    EGLContextState egl;
    if (!initEGL(egl)) {
        std::cerr << "[GPU] Failed to initialize EGL context\n";
        return -1;
    }

    //std::cout << "[GPU] === Workload start: level = " << level << " ===\n";
    run_gpu_compute_workload(static_cast<float>(level));

    cleanupEGL(egl);
    //std::cout << "[GPU] === Workload complete ===\n";
    return 0;
}

