#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <algorithm>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

bool initEGL() {
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        std::cerr << "Failed to get EGL display." << std::endl;
        return false;
    }

    if (!eglInitialize(display, nullptr, nullptr)) {
        std::cerr << "Failed to initialize EGL." << std::endl;
        return false;
    }

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8, EGL_NONE
    };
    EGLConfig config;
    EGLint numConfigs;
    if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs)) {
        std::cerr << "Failed to choose EGL config." << std::endl;
        return false;
    }

    EGLint pbufferAttribs[] = {
        EGL_WIDTH, 1024,
        EGL_HEIGHT, 1024,
        EGL_NONE,
    };
    EGLSurface surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
    if (surface == EGL_NO_SURFACE) {
        std::cerr << "Failed to create EGL surface." << std::endl;
        return false;
    }

    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        std::cerr << "Failed to create EGL context." << std::endl;
        return false;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        std::cerr << "Failed to make EGL context current." << std::endl;
        return false;
    }

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "[GL] Renderer: " << renderer << "\n[GL] Version : " << version << std::endl;

    return true;
}

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
    std::this_thread::sleep_for(std::chrono::seconds(30));

    for (auto tex : textures) {
        glDeleteTextures(1, &tex);
    }

    std::cout << "Cleaned up GPU textures." << std::endl;
}

int main(int argc, char** argv) {
    int tex_size = 4096;
    int num_textures = 6;
    int delay_ms = 100;

    if (argc != 4) {
        std::cout << "Usage: " << argv[0] << " <tex_size> <num_textures> <delay_ms>\n";
        std::cout << "Example: " << argv[0] << " 4096 6 100\n";
        return 1;
    }

    tex_size = std::atoi(argv[1]);
    num_textures = std::atoi(argv[2]);
    delay_ms = std::atoi(argv[3]);

    if (!initEGL()) return 1;

    run_gpu_texture_workload(tex_size, num_textures, delay_ms);

    return 0;
}

