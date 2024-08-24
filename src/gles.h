#pragma once

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

extern EGLDisplay egl_display;
extern EGLSurface egl_surface;
extern EGLContext egl_context;
extern EGLConfig egl_config;

bool InitOGLESContext();

bool DestroyOGLESContext();