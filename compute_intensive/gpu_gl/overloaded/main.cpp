#include <unistd.h>
#include <iostream>
#include <cstdlib>      // for atoi
#include "egl_util.h"

void run_gpu_compute_workload(float base_level, float duration_sec);

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "<usage> : ./gl_workload <level> <duration>\n";
        return 1;
    }

    int level = std::atoi(argv[1]);
    int duration_sec = std::atoi(argv[2]);

    if (level <= 0) {
        std::cerr << "Error: level must be a positive integer.\n";
        return 1;
    }

    EGLContextState egl;
    if (!initEGL(egl)) {
        std::cerr << "[GPU] Failed to initialize EGL context\n";
        return -1;
    }

    //std::cout << "[GPU] === Workload start: level = " << level << " ===\n";
    run_gpu_compute_workload(static_cast<float>(level), static_cast<float>(duration_sec));

    cleanupEGL(egl);
    //std::cout << "[GPU] === Workload complete ===\n";
    return 0;
}

