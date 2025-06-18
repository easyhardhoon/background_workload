#include "egl_util.h"
#include <iostream>
#include <X11/Xlib.h>
#include <EGL/eglext.h>


bool initEGL(EGLContextState& egl) {
    egl.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (egl.display == EGL_NO_DISPLAY) {
        std::cerr << "[EGL] eglGetDisplay returned EGL_NO_DISPLAY." << std::endl;
        return false;
    }

    if (!eglInitialize(egl.display, nullptr, nullptr)) {
        std::cerr << "[EGL] eglInitialize failed." << std::endl;
        return false;
    }

    const EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    EGLint num_configs;
    if (!eglChooseConfig(egl.display, config_attribs, &egl.config, 1, &num_configs)) {
        std::cerr << "[EGL] eglChooseConfig failed." << std::endl;
        return false;
    }

    const EGLint pbuffer_attribs[] = {
        EGL_WIDTH, 1024,
        EGL_HEIGHT, 1024,
        EGL_NONE
    };
    egl.surface = eglCreatePbufferSurface(egl.display, egl.config, pbuffer_attribs);
    if (egl.surface == EGL_NO_SURFACE) {
        std::cerr << "[EGL] eglCreatePbufferSurface failed." << std::endl;
        return false;
    }

    const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    egl.context = eglCreateContext(egl.display, egl.config, EGL_NO_CONTEXT, context_attribs);
    if (egl.context == EGL_NO_CONTEXT) {
        std::cerr << "[EGL] eglCreateContext failed." << std::endl;
        return false;
    }

    if (!eglMakeCurrent(egl.display, egl.surface, egl.surface, egl.context)) {
        std::cerr << "[EGL] eglMakeCurrent failed." << std::endl;
        return false;
    }

    std::cout << "[EGL] Successfully initialized EGL context." << std::endl;
    return true;
}

void cleanupEGL(EGLContextState& egl) {
    eglMakeCurrent(egl.display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(egl.display, egl.context);
    eglDestroySurface(egl.display, egl.surface);
    eglTerminate(egl.display);
}

