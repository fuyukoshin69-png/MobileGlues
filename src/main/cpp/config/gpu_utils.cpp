//
// Created by hanji on 2025/2/9.
//

#include "gpu_utils.h"
#include "../gles/loader.h"
#include "vulkan/vulkan.h"

#include <EGL/egl.h>
#include <cstring>

static const char *gles3_lib[] = {
        "libGLESv3_CM",
        "libGLESv3",
        nullptr
};

const char* getGPUInfo() {
    EGLDisplay eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY || eglInitialize(eglDisplay, nullptr, nullptr) != EGL_TRUE)
        return nullptr;

    EGLint egl_attributes[] = {
            EGL_BLUE_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_RED_SIZE, 8,
            EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 24, EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL_NONE
    };

    EGLint num_configs = 0;
    if (eglChooseConfig(eglDisplay, egl_attributes, nullptr, 0, &num_configs) != EGL_TRUE || num_configs == 0) {
        eglTerminate(eglDisplay);
        return nullptr;
    }

    EGLConfig eglConfig;
    eglChooseConfig(eglDisplay, egl_attributes, &eglConfig, 1, &num_configs);

    const EGLint egl_context_attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    EGLContext context = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, egl_context_attributes);
    if (context == EGL_NO_CONTEXT) {
        eglTerminate(eglDisplay);
        return nullptr;
    }

    if (eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, context) != EGL_TRUE) {
        eglDestroyContext(eglDisplay, context);
        eglTerminate(eglDisplay);
        return nullptr;
    }

    const char* renderer = nullptr;
    void* lib = open_lib(gles3_lib, nullptr);
    if (lib) {
        GLES.glGetString = (const GLubyte * (*)( GLenum ))dlsym(lib, "glGetString");
        if (GLES.glGetString) {
            renderer = (const char*)GLES.glGetString(GL_RENDERER);
        }
    }

    eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(eglDisplay, context);
    eglTerminate(eglDisplay);

    dlclose(lib);

    return renderer;
}

int isAdreno(const char* gpu) {
//    const char* gpu = getGPUInfo();
    if (!gpu)
        return 0;
    return strstr(gpu, "Adreno") != nullptr;
}

int isAdreno740(const char* gpu) {
//    const char* gpu = getGPUInfo();
    if (!gpu)
        return 0;
    return isAdreno(gpu) && (strstr(gpu, "740") != nullptr);
}

int isAdreno830(const char* gpu) {
//    const char* gpu = getGPUInfo();
    if (!gpu)
        return 0;
    return isAdreno(gpu) && (strstr(gpu, "830") != nullptr);
}