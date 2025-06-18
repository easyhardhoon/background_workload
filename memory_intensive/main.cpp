#include <iostream>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

extern void run_gpu_texture_workload(int tex_size, int num_textures, int delay_ms);

bool initEGL() {
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        std::cerr << "Failed to get EGL display" << std::endl;
        return false;
    }

    if (!eglInitialize(display, nullptr, nullptr)) {
        std::cerr << "Failed to initialize EGL" << std::endl;
        return false;
    }

    EGLint attribs[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLConfig config;
    EGLint numConfigs;
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    EGLint ctx_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctx_attribs);
    if (context == EGL_NO_CONTEXT) {
        std::cerr << "Failed to create EGL context" << std::endl;
        return false;
    }

    EGLint pbuffer_attribs[] = {
        EGL_WIDTH, 16,
        EGL_HEIGHT, 16,
        EGL_NONE,
    };
    surface = eglCreatePbufferSurface(display, config, pbuffer_attribs);
    if (surface == EGL_NO_SURFACE) {
        std::cerr << "Failed to create EGL surface" << std::endl;
        return false;
    }

    if (!eglMakeCurrent(display, surface, surface, context)) {
        std::cerr << "Failed to make EGL context current" << std::endl;
        return false;
    }

    std::cout << "EGL initialized." << std::endl;
    return true;
}

int main() {
    if (!initEGL()) return 1;

    //run_gpu_texture_workload(4096, 8, 100);  // texture size, count, delay(ms)
    run_gpu_texture_workload(4096, 6, 100);  // texture size, count, delay(ms)
    //run_gpu_texture_workload(4096, 2, 100);  // texture size, count, delay(ms)
    //run_gpu_texture_workload(4096, 1, 100);  // texture size, count, delay(ms)

    return 0;
}

