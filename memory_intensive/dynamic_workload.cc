#include <GLES2/gl2.h>
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>

void run_gpu_texture_workload(int tex_size, int num_textures, int delay_ms) {
    std::vector<GLuint> textures;

    std::cout << "Allocating " << num_textures << " GPU textures of size " 
              << tex_size << "x" << tex_size << " RGBA(FLOAT32)..." << std::endl;

    for (int i = 0; i < num_textures; ++i) {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // 실제 GPU VRAM 사용 유도
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_size, tex_size, 0, GL_RGBA, GL_FLOAT, nullptr);

        GLenum err = glGetError();
        if (err != GL_NO_ERROR) {
            std::cerr << "OpenGL Error at texture " << i << ": 0x" << std::hex << err << std::endl;
            break;
        }

        textures.push_back(tex);
        std::cout << "Allocated texture " << i + 1 << "/" << num_textures << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }

    std::cout << "Workload complete. Holding textures for 30 sec..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));  // 점유 유지

    for (auto tex : textures) {
        glDeleteTextures(1, &tex);
    }

    std::cout << "Cleaned up GPU textures." << std::endl;
}

