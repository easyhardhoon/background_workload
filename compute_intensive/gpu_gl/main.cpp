#include <unistd.h>
#include <iostream>
#include <cstdlib>      // for atoi
#include "egl_util.h"

void run_gpu_compute_workload(int duration_sec, int level);

int main(int argc, char** argv) {
    int duration_sec = 10;
    int level = 10;

    if (argc!=3){
//     	std::cout << "<usage> : ./gl_workload <duration> <level\n";
	return 1;
    }
    if (argc >= 2) duration_sec = std::atoi(argv[1]);
    if (argc >= 3) level = std::atoi(argv[2]);

// std::cout << "=== Initial sleep (5s) ===\n";
    sleep(5);

    EGLContextState egl;
    if (!initEGL(egl)) {
        return -1;
    }

// std::cout << "=== GPU workload start: duration=" << duration_sec
//             << "s, level=" << level << " ===\n";

    run_gpu_compute_workload(duration_sec, level);

    cleanupEGL(egl);
    return 0;
}

