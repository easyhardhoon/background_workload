#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>

#include <CL/cl.h>
#define CORAL

int main(int argc, char** argv) {
    size_t buffer_size_mb = 64;
    int num_buffers = 4;
    int hold_sec = 30;

    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <buffer_size_MB> <num_buffers> <hold_sec>\n";
        std::cout << "Example: " << argv[0] << " 64 4 30\n";
        return 1;
    }

    buffer_size_mb = std::atoi(argv[1]);
    num_buffers = std::atoi(argv[2]);
    hold_sec = std::atoi(argv[3]);

    cl_int err;

    cl_platform_id platform;
    err = clGetPlatformIDs(1, &platform, nullptr);
    if (err != CL_SUCCESS) {
        std::cerr << "clGetPlatformIDs failed: " << err << std::endl;
        return 1;
    }

    cl_device_id device;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
    if (err != CL_SUCCESS) {
        std::cerr << "clGetDeviceIDs failed: " << err << std::endl;
        return 1;
    }

    cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "clCreateContext failed: " << err << std::endl;
        return 1;
    }

#ifdef CORAL
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
#else
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);
#endif
    if (err != CL_SUCCESS) {
        std::cerr << "clCreateCommandQueue failed: " << err << std::endl;
        return 1;
    }

    size_t buffer_bytes = buffer_size_mb * 1024 * 1024;
    std::vector<cl_mem> buffers;
    std::vector<char> pattern(buffer_bytes, static_cast<char>(0xA5));

    for (int i = 0; i < num_buffers; ++i) {
        cl_mem buf = clCreateBuffer(context, CL_MEM_READ_WRITE, buffer_bytes, nullptr, &err);
        if (err != CL_SUCCESS) {
            std::cerr << "Buffer " << i + 1 << " creation failed: " << err << std::endl;
            continue;  
        }

        err = clEnqueueWriteBuffer(queue, buf, CL_TRUE, 0, buffer_bytes, pattern.data(), 0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Buffer " << i + 1 << " write failed: " << err << std::endl;
            clReleaseMemObject(buf);
            continue;
        }

        buffers.push_back(buf);
        std::cout << "Allocated buffer " << i + 1 << "/" << num_buffers << " (" << buffer_size_mb << " MB)" << std::endl;
    }

    std::cout << "Holding buffers for " << hold_sec << " seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(hold_sec));

    for (auto buf : buffers) {
        clReleaseMemObject(buf);
    }

    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    std::cout << "Completed GPU memory workload and cleaned up." << std::endl;
    return 0;
}

