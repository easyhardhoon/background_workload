#ifndef EGL_UTIL_H
#define EGL_UTIL_H

#include <EGL/egl.h>

struct EGLContextState {
    EGLDisplay display;
    EGLContext context;
    EGLSurface surface;
    EGLConfig config;
};

bool initEGL(EGLContextState& egl);
void cleanupEGL(EGLContextState& egl);

#endif

