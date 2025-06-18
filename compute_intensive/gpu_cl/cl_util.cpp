#include "cl_util.h"
#include <iostream>

bool initCL(CLContextState& cl) {
    cl_int err;

    err = clGetPlatformIDs(1, &cl.platform, nullptr);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to get platform ID: " << getCLErrorString(err) << std::endl;
        return false;
    }

    err = clGetDeviceIDs(cl.platform, CL_DEVICE_TYPE_GPU, 1, &cl.device, nullptr);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to get GPU device: " << getCLErrorString(err) << std::endl;
        return false;
    }

    cl.context = clCreateContext(nullptr, 1, &cl.device, nullptr, nullptr, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create context: " << getCLErrorString(err) << std::endl;
        return false;
    }

    cl.queue = clCreateCommandQueueWithProperties(cl.context, cl.device, 0, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Failed to create command queue: " << getCLErrorString(err) << std::endl;
        return false;
    }

    return true;
}

void cleanupCL(CLContextState& cl) {
    if (cl.queue) clReleaseCommandQueue(cl.queue);
    if (cl.context) clReleaseContext(cl.context);
}

std::string getCLErrorString(cl_int err) {
    switch (err) {
        case CL_SUCCESS: return "CL_SUCCESS";
        case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
        case CL_DEVICE_NOT_AVAILABLE: return "CL_DEVICE_NOT_AVAILABLE";
        case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
        case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
        default: return "Unknown Error";
    }
}

