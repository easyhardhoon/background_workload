#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <thread>
#include <random>   
#include <algorithm>

#define MODERATE
//#define OVERLOADED

GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compile error: " << log << std::endl;
        return 0;
    }
    return shader;
}

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
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
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

    return true;
}

void run_gpu_compute_workload(float base_level) {
    const int loop_count = 1;
    const int phase_count = 4;
    const float phase_duration_sec = 5.0f;
    const float job_time_sec = 4.0f;
    const int phase_interval_ms = 1000;
    const int job_interval_ms = 1;

    std::string frag_code = R"(
        precision mediump float;
        void main() {
            float x = 0.0;
            for (int i = 0; i < )" + std::to_string(loop_count) + R"(; ++i) {
                float f = float(i);
                x += sin(f) * cos(f) + log(f + 1.0) * sqrt(f + 0.1) + tan(f);
            }
            gl_FragColor = vec4(x);
        }
    )";

    const char* vtx = R"(
        attribute vec2 pos;
        void main() {
            gl_Position = vec4(pos, 0.0, 1.0);
        }
    )";

    GLuint vs = compileShader(GL_VERTEX_SHADER, vtx);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, frag_code.c_str());

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glBindAttribLocation(prog, 0, "pos");
    glLinkProgram(prog);
    glUseProgram(prog);

    float verts[] = {
        -1.f, -1.f, 1.f, -1.f, -1.f, 1.f,
         1.f, -1.f, 1.f,  1.f, -1.f, 1.f
    };

    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    GLuint fbo, tex;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8192, 8192, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
        return;
    }

    std::vector<int> level_seq;

#ifdef MODERATE
    for (int i = 1; i <= phase_count; ++i) {
        level_seq.push_back(std::max(1, static_cast<int>(base_level * i / phase_count)));
    }
#endif
#ifdef OVERLOADED
    for (int i = 1; i <= phase_count; ++i) {
        level_seq.push_back(static_cast<int>(base_level * i));
    }
#endif

    std::mt19937 rng(42);
    std::shuffle(level_seq.begin(), level_seq.end(), rng);

    int total_job_count = 0;

    for (int phase = 0; phase < phase_count; ++phase) {
        int level = level_seq[phase];
        auto phase_start = std::chrono::steady_clock::now();

        std::cout << "[GPU Phase " << (phase + 1) << "] Start - Level: " << level
                  << ", Duration: " << phase_duration_sec << "s\n";

        auto job_start = std::chrono::steady_clock::now();
        while (true) {
            auto now = std::chrono::steady_clock::now();
            float elapsed = std::chrono::duration<float>(now - job_start).count();
            if (elapsed >= job_time_sec) break;

            for (int i = 0; i < level; ++i) {
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }
            glFinish();
            total_job_count++;

            std::this_thread::sleep_for(std::chrono::milliseconds(job_interval_ms));
        }

        if (phase < phase_count - 1) {
            std::cout << "[GPU Phase " << (phase + 1) << "] Sleeping " << phase_interval_ms << "ms between phases\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(phase_interval_ms));
        }
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
}

