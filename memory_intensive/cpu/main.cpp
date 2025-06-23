#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>

void run_cpu_memory_workload(int num_blocks, size_t block_size_mb, int hold_duration_sec) {
    const size_t block_bytes = block_size_mb * 1024 * 1024;

    std::cout << "Allocating " << num_blocks << " blocks of "
              << block_size_mb << " MB each ("
              << (num_blocks * block_size_mb) << " MB total)..." << std::endl;

    std::vector<char*> memory_blocks;

    for (int i = 0; i < num_blocks; ++i) {
        char* block = new(std::nothrow) char[block_bytes];
        if (!block) {
            std::cerr << "Memory allocation failed at block " << i << std::endl;
            break;
        }

        // Touch the memory to commit pages
        std::memset(block, 0xA5, block_bytes);

        memory_blocks.push_back(block);
        std::cout << "Allocated block " << i + 1 << "/" << num_blocks << std::endl;
    }

    std::cout << "Holding memory for " << hold_duration_sec << " seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(hold_duration_sec));

    for (auto block : memory_blocks) {
        delete[] block;
    }

    std::cout << "Memory released.\n";
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <num_blocks> <block_size_MB> <hold_sec>\n";
        std::cout << "Example: " << argv[0] << " 5 512 30\n";
        return 1;
    }

    int num_blocks = std::atoi(argv[1]);
    int block_size_mb = std::atoi(argv[2]);
    int hold_duration_sec = std::atoi(argv[3]);

    if (num_blocks <= 0 || block_size_mb <= 0 || hold_duration_sec <= 0) {
        std::cerr << "Invalid input values.\n";
        return 1;
    }

    run_cpu_memory_workload(num_blocks, block_size_mb, hold_duration_sec);

    return 0;
}

