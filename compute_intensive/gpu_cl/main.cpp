#include "cl_util.h"
#include <iostream>

extern void run_cl_workload(CLContextState& cl, int duration_sec, int level);

int main(int argc, char** argv) {
    int duration_sec = 10;
    int level = 10; // default to maximum workload
    if (argc!=3){
    	std::cout << "<usage> : ./cl_workload <duration> <level>\n";
	return 1;
    }
    if (argc >= 2) duration_sec = std::atoi(argv[1]);
    if (argc >= 3) level = std::atoi(argv[2]);

    CLContextState cl;
    if (!initCL(cl)) {
        std::cerr << "Failed to initialize OpenCL." << std::endl;
        return -1;
    }

    run_cl_workload(cl, duration_sec, level);

    cleanupCL(cl);
    return 0;
}

