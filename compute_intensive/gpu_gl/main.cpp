#include "egl_util.h"

void run_gpu_compute_workload(int duration_sec, int level);

int main() {
    EGLContextState egl;
    if (!initEGL(egl)) {
        return -1;
    }

    run_gpu_compute_workload(30,3);
    //run_gpu_compute_workload(30,7);

    cleanupEGL(egl);
    return 0;
}

