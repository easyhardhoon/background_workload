#include "cl_util.h"
#include <unistd.h>
#include <cstdlib>
#include <iostream>

extern void run_cl_workload(CLContextState& cl, int level, int duration_sec);

int main(int argc, char** argv) {
    //freopen("/dev/null", "w", stderr); 
    //freopen("/dev/null", "w", stdout); 
    
    int duration = 10;
    int level = 10; // default to maximum workload
    if (argc!=3){
    	std::cout << "<usage> : ./cl_workload <level> <duration>\n";
	return 1;
    }
    if (argc >= 2) level = std::atoi(argv[1]);
    if (argc >= 3) duration = std::atoi(argv[2]);
    
    CLContextState cl;
    if (!initCL(cl)) {
        std::cerr << "Failed to initialize OpenCL." << std::endl;
        return -1;
    }
    run_cl_workload(cl, level, duration);

    cleanupCL(cl);
    return 0;
}

