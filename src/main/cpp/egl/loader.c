//
// Created by Swung0x48 on 2024/10/10.
//

#include <EGL/egl.h>
#include <string.h>
#include "loader.h"
#include "../includes.h"
#include "../gl/log.h"
#include "../gl/envvars.h"
#include "../gles/loader.h"

#define DEBUG 0

struct egl_func_t g_egl_func;

void init_target_egl() {
    LOG_D("Initializing %s @ %s", RENDERERNAME, __FUNCTION__);
    EGLDisplay eglDisplay = EGL_NO_DISPLAY;
    EGLSurface eglSurface = EGL_NO_SURFACE;
    EGLContext eglContext = EGL_NO_CONTEXT;

    INIT_EGL_FUNC(eglGetProcAddress);
    INIT_EGL_FUNC(eglBindAPI);
    INIT_EGL_FUNC(eglInitialize);
    INIT_EGL_FUNC(eglGetDisplay);
    INIT_EGL_FUNC(eglCreatePbufferSurface);
    INIT_EGL_FUNC(eglDestroySurface);
    INIT_EGL_FUNC(eglDestroyContext);
    INIT_EGL_FUNC(eglMakeCurrent);
    INIT_EGL_FUNC(eglChooseConfig);
    INIT_EGL_FUNC(eglCreateContext);
    INIT_EGL_FUNC(eglQueryString);
    INIT_EGL_FUNC(eglTerminate);
    INIT_EGL_FUNC(eglGetError);

    LOAD_EGL_FUNC(eglGetProcAddress);
    LOAD_EGL_FUNC(eglBindAPI);
    LOAD_EGL_FUNC(eglInitialize);
    LOAD_EGL_FUNC(eglGetDisplay);
    LOAD_EGL_FUNC(eglCreatePbufferSurface);
    LOAD_EGL_FUNC(eglDestroySurface);
    LOAD_EGL_FUNC(eglDestroyContext);
    LOAD_EGL_FUNC(eglMakeCurrent);
    LOAD_EGL_FUNC(eglChooseConfig);
    LOAD_EGL_FUNC(eglCreateContext);
    LOAD_EGL_FUNC(eglQueryString);
    LOAD_EGL_FUNC(eglTerminate);
    LOAD_EGL_FUNC(eglGetError);

    eglDisplay = egl_eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (eglDisplay == EGL_NO_DISPLAY) {
        LOG_E("eglGetDisplay failed (0x%x)", egl_eglGetError());
        goto cleanup;
    }

    if (egl_eglInitialize(eglDisplay, NULL, NULL) != EGL_TRUE) {
        LOG_E("eglInitialize failed (0x%x)", egl_eglGetError());
        goto cleanup;
    }

    if (egl_eglBindAPI(EGL_OPENGL_ES_API) != EGL_TRUE) {
        LOG_E("eglBindAPI failed (0x%x)", egl_eglGetError());
        goto cleanup;
    }

    EGLint configAttribs[] = {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
    };

    EGLConfig pbufConfig;
    EGLint configsFound = 0;
    if (egl_eglChooseConfig(eglDisplay, configAttribs, &pbufConfig, 1, &configsFound) != EGL_TRUE) {
        LOG_E("eglChooseConfig failed (0x%x)", egl_eglGetError());
        goto cleanup;
    }

    if (configsFound == 0) {
        configAttribs[6] = 0;
        if (egl_eglChooseConfig(eglDisplay, configAttribs, &pbufConfig, 1, &configsFound) != EGL_TRUE) {
            LOG_E("Retry eglChooseConfig failed (0x%x)", egl_eglGetError());
            goto cleanup;
        }
        if (configsFound) {
            LOG_I("Using config without alpha channel");
        } else {
            LOG_E("No valid EGL config found");
            goto cleanup;
        }
    }

    EGLint ctxAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
    eglContext = egl_eglCreateContext(eglDisplay, pbufConfig, EGL_NO_CONTEXT, ctxAttribs);
    if (eglContext == EGL_NO_CONTEXT) {
        LOG_E("eglCreateContext failed (0x%x)", egl_eglGetError());
        goto cleanup;
    }

    EGLint pbAttribs[] = { EGL_WIDTH, 32, EGL_HEIGHT, 32, EGL_NONE };
    eglSurface = egl_eglCreatePbufferSurface(eglDisplay, pbufConfig, pbAttribs);
    if (eglSurface == EGL_NO_SURFACE) {
        LOG_E("eglCreatePbufferSurface failed (0x%x)", egl_eglGetError());
        goto cleanup;
    }

    if (egl_eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext) != EGL_TRUE) {
        LOG_E("eglMakeCurrent failed (0x%x)", egl_eglGetError());
        goto cleanup;
    }

    LOG_V("EGL initialized successfully");
    return;

cleanup:
    if (eglSurface != EGL_NO_SURFACE) {
        egl_eglDestroySurface(eglDisplay, eglSurface);
    }
    if (eglContext != EGL_NO_CONTEXT) {
        egl_eglDestroyContext(eglDisplay, eglContext);
    }
    if (eglDisplay != EGL_NO_DISPLAY) {
        egl_eglTerminate(eglDisplay);
    }
    LOG_E("EGL initialization failed");
}