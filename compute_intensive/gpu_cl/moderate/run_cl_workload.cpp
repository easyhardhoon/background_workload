#include <cstdio>
#include "cl_util.h"
#include <iostream>
#include <chrono>
#include <thread>

#define MODERATE
//#define OVERLOADED


void run_cl_workload(CLContextState& cl, int level, int duration_sec) {
#ifdef MODERATE
    const int loop_count = level/2;
#endif

#ifdef OVERLOADED
    const int loop_count = 2 * level;
#endif
    const int job_interval_ms = 1;

    std::string kernel_src = R"CLC(
    __kernel void compute_heavy(__global float* output) {
        int id = get_global_id(0);
        float x = 0.0f;
        for (int i = 0; i < LOOP_COUNT; ++i) {
            x += sin((float)i) * cos((float)i);
        }
        output[id] = x;
    }
    )CLC";

    // Replace LOOP_COUNT at runtime
    std::string loop_define = std::to_string(loop_count);
    size_t pos = kernel_src.find("LOOP_COUNT");
    kernel_src.replace(pos, std::string("LOOP_COUNT").length(), loop_define);

    cl_int err;
    const char* src = kernel_src.c_str();
    cl_program program = clCreateProgramWithSource(cl.context, 1, &src, nullptr, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create program: " << getCLErrorString(err) << std::endl;
        return;
    }

    err = clBuildProgram(program, 1, &cl.device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to build program\n";
        return;
    }

    cl_kernel kernel = clCreateKernel(program, "compute_heavy", &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create kernel\n";
        return;
    }

    size_t global_size = 1024 * 1024;
    cl_mem output = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, sizeof(float) * global_size, nullptr, &err);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &output);

    std::cout << "[CL] Running workload for " << duration_sec
              << " sec at level " << level << " (loop count = " << loop_count << ")\n";

    auto start = std::chrono::steady_clock::now();
    int iteration_count = 0;

    while (true) {
        auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - start).count();
        if (elapsed >= duration_sec) break;

        err = clEnqueueNDRangeKernel(cl.queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
        clFinish(cl.queue);
        ++iteration_count;

        std::this_thread::sleep_for(std::chrono::milliseconds(job_interval_ms));
    }

    std::cout << "[CL] Job complete. Total iterations: " << iteration_count << "\n";

    clReleaseMemObject(output);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
}

