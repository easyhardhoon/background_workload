#pragma once
#include <CL/cl.h>
#include <string>

struct CLContextState {
    cl_platform_id platform = nullptr;
    cl_device_id device = nullptr;
    cl_context context = nullptr;
    cl_command_queue queue = nullptr;
};

bool initCL(CLContextState& cl);
void cleanupCL(CLContextState& cl);
std::string getCLErrorString(cl_int err);

