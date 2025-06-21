#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>

void run_cpu_memory_workload(size_t mem_size_mb, int block_size_kb, int delay_ms) {
    const size_t total_bytes = mem_size_mb * 1024 * 1024;
    const size_t block_bytes = block_size_kb * 1024;
    const size_t num_blocks = total_bytes / block_bytes;

    std::cout << "Allocating " << mem_size_mb << " MB of memory in "
              << num_blocks << " blocks (" << block_size_kb << " KB each)..." << std::endl;

    std::vector<char*> memory_blocks;

    for (size_t i = 0; i < num_blocks; ++i) {
        char* block = new(std::nothrow) char[block_bytes];
        if (!block) {
            std::cerr << "Memory allocation failed at block " << i << std::endl;
            break;
        }

        // Optional: Touch memory to ensure allocation isn't lazy
        std::memset(block, i % 256, block_bytes);

        memory_blocks.push_back(block);

        std::cout << "Allocated block " << i + 1 << "/" << num_blocks << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    std::cout << "Memory workload complete. Holding memory for 30 seconds..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));

    for (auto block : memory_blocks) {
        delete[] block;
    }

    std::cout << "Cleaned up memory." << std::endl;
}

int main(int argc, char** argv) {
    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <mem_MB> <block_KB> <delay_ms>\n";
        std::cout << "Example: " << argv[0] << " 2048 512 50\n";
        return 1;
    }

    size_t mem_size_mb = std::atoi(argv[1]);
    int block_size_kb = std::atoi(argv[2]);
    int delay_ms = std::atoi(argv[3]);

    if (mem_size_mb <= 0 || block_size_kb <= 0 || delay_ms < 0) {
        std::cerr << "Invalid input values.\n";
        return 1;
    }

    run_cpu_memory_workload(mem_size_mb, block_size_kb, delay_ms);

    return 0;
}

