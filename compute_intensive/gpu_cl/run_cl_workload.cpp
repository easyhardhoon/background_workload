#include "cl_util.h"
#include <iostream>
#include <chrono>

void run_cl_workload(CLContextState& cl, int duration_sec, int level) {
    // Clamp level between 1 and 10
    level = std::max(1, std::min(level, 10));
    int loop_count = 1000 * level;

    std::string kernel_src = R"CLC(
    __kernel void compute_heavy(__global float* output, int loop_count) {
        int id = get_global_id(0);
        float x = 0.0f;
        for (int i = 0; i < LOOP_COUNT; ++i) {
            x += sin((float)i) * cos((float)i);
        }
        output[id] = x;
    }
    )CLC";

    // Replace LOOP_COUNT at runtime with actual number
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
        std::cerr << "Failed to build program" << std::endl;
        return;
    }

    cl_kernel kernel = clCreateKernel(program, "compute_heavy", &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create kernel" << std::endl;
        return;
    }

    size_t global_size = 1024 * 1024;
    cl_mem output = clCreateBuffer(cl.context, CL_MEM_WRITE_ONLY, sizeof(float) * global_size, nullptr, &err);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &output);
    clSetKernelArg(kernel, 1, sizeof(int), &loop_count);

    std::cout << "Running CL workload for " << duration_sec << " sec at level " << level << "...\n";
    auto start = std::chrono::steady_clock::now();

    while (true) {
        err = clEnqueueNDRangeKernel(cl.queue, kernel, 1, nullptr, &global_size, nullptr, 0, nullptr, nullptr);
        clFinish(cl.queue);

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() >= duration_sec)
            break;
    }

    std::cout << "CL workload complete.\n";

    clReleaseMemObject(output);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
}

