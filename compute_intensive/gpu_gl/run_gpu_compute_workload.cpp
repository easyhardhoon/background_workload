#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <iostream>
#include <vector>
#include <chrono>

GLuint compileShader(GLenum type, const char* src) {
    std::cout << "Compiling shader of type: " << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << std::endl;
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
    std::cout << "[EGL] Getting EGL display..." << std::endl;
    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
        std::cerr << "Failed to get EGL display." << std::endl;
        return false;
    }

    std::cout << "[EGL] Initializing EGL..." << std::endl;
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
    std::cout << "[EGL] Choosing EGL config..." << std::endl;
    if (!eglChooseConfig(display, configAttribs, &config, 1, &numConfigs)) {
        std::cerr << "Failed to choose EGL config." << std::endl;
        return false;
    }

    EGLint pbufferAttribs[] = {
        EGL_WIDTH, 1024,
        EGL_HEIGHT, 1024,
        EGL_NONE,
    };
    std::cout << "[EGL] Creating PBuffer surface..." << std::endl;
    EGLSurface surface = eglCreatePbufferSurface(display, config, pbufferAttribs);
    if (surface == EGL_NO_SURFACE) {
        std::cerr << "Failed to create EGL surface." << std::endl;
        return false;
    }

    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    std::cout << "[EGL] Creating context..." << std::endl;
    EGLContext context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        std::cerr << "Failed to create EGL context." << std::endl;
        return false;
    }

    std::cout << "[EGL] Making context current..." << std::endl;
    if (!eglMakeCurrent(display, surface, surface, context)) {
        std::cerr << "Failed to make EGL context current." << std::endl;
        return false;
    }

    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "[GL] Renderer: " << renderer << std::endl;
    std::cout << "[GL] Version : " << version << std::endl;

    return true;
}

void run_gpu_compute_workload(int duration_sec, int intensity_level) {
    int loop_count = intensity_level * 5;
    std::string frag_code = R"(
        precision mediump float;
        void main() {
            float x = 0.0;
            for (int i = 0; i < )" + std::to_string(loop_count) + R"(; ++i) {
                x += sin(float(i)) * cos(float(i));
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

    std::cout << "[GL] Linking program..." << std::endl;
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glBindAttribLocation(prog, 0, "pos");
    glLinkProgram(prog);
    glUseProgram(prog);

    std::cout << "[GL] Creating VBO..." << std::endl;
    float verts[] = {
        -1.f, -1.f, 1.f, -1.f, -1.f, 1.f,
        1.f, -1.f, 1.f, 1.f, -1.f, 1.f
    };
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    std::cout << "[GL] Setting up framebuffer..." << std::endl;
    GLuint fbo, tex;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1024, 1024, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer not complete!" << std::endl;
        return;
    }

    std::cout << "[GL] Running workload with intensity level " << intensity_level << std::endl;
    auto start = std::chrono::steady_clock::now();

    while (true) {
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glFinish();

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() >= duration_sec)
            break;
    }

    std::cout << "GPU compute workload complete." << std::endl;

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
}

