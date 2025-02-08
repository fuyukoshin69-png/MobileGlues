//
// Created by Swung0x48 on 2024/10/10.
//

#include "egl.h"
#include "../includes.h"

#define DEBUG 0

extern GLuint watermark_vao;
extern GLuint watermark_vbo;
extern GLuint watermark_program;

EGLBoolean eglSwapBuffers(EGLDisplay dpy, EGLSurface surface) {
    LOG()

    LOAD_EGL_FUNC(eglSwapBuffers)

    LOG_D("Drawing watermark!")
    LOAD_GLES_FUNC(glGetIntegerv)
    LOAD_GLES_FUNC(glBindVertexArray)
    LOAD_GLES_FUNC(glBindBuffer)
    LOAD_GLES_FUNC(glUseProgram)
    LOAD_GLES_FUNC(glDrawArrays)
    LOAD_GLES_FUNC(glEnableVertexAttribArray)

    INIT_CHECK_GL_ERROR

    // Save states
    GLint prev_vao;
    GLint prev_vbo;
    GLint prev_prog;
    gles_glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &prev_vao);
    CHECK_GL_ERROR_NO_INIT
    gles_glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &prev_vbo);
    CHECK_GL_ERROR_NO_INIT
    gles_glGetIntegerv(GL_CURRENT_PROGRAM, &prev_prog);
    CHECK_GL_ERROR_NO_INIT

    LOG_D("prev vao: %d", prev_vao);
    LOG_D("prev vbo: %d", prev_vbo);
    LOG_D("prev program: %d", prev_prog);

    LOG_D("Save ok")

    // Set states
    gles_glBindVertexArray(watermark_vao);
    CHECK_GL_ERROR_NO_INIT
//        gles_glEnableVertexAttribArray(0);
    gles_glBindBuffer(GL_ARRAY_BUFFER, watermark_vbo);
//        CHECK_GL_ERROR_NO_INIT
    gles_glUseProgram(watermark_program);
    CHECK_GL_ERROR_NO_INIT
    LOG_D("set ok")


    LOG_D("Watermark vao: %d", watermark_vao);
    LOG_D("Watermark vbo: %d", watermark_vbo);
    LOG_D("Watermark program: %d", watermark_program);

    LOG_D("gles_glDrawArrays: 0x%x", *gles_glDrawArrays);
    // Draw!
    gles_glDrawArrays(GL_TRIANGLES, 0, 3);
    CHECK_GL_ERROR_NO_INIT
    LOG_D("draw ok")


    // Restore states
    gles_glBindVertexArray(prev_vao);
    CHECK_GL_ERROR_NO_INIT
    gles_glBindBuffer(GL_ARRAY_BUFFER, prev_vbo);
    CHECK_GL_ERROR_NO_INIT
    gles_glUseProgram(prev_prog);
    CHECK_GL_ERROR_NO_INIT
    LOG_D("restore ok")

    EGLBoolean ret = egl_eglSwapBuffers(dpy, surface);
    return ret;
}

//EGLContext mglues_eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) {
//    __android_log_print(ANDROID_LOG_VERBOSE, RENDERERNAME,
//                        "%s @ %s(...)", RENDERERNAME, __FUNCTION__);
//
//    EGLContext ctx = g_egl_func.eglCreateContext(dpy, config, share_context, attrib_list);
//
//    return ctx;
//}
//
//EGLBoolean mglues_eglDestroyContext(EGLDisplay dpy, EGLContext ctx) {
//    __android_log_print(ANDROID_LOG_VERBOSE, RENDERERNAME,
//                        "%s @ %s(...)", RENDERERNAME, __FUNCTION__);
//    return g_egl_func.eglDestroyContext(dpy, ctx);
////    return EGL_TRUE;
//}
//
//EGLBoolean mglues_eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx) {
//    __android_log_print(ANDROID_LOG_VERBOSE, RENDERERNAME,
//                        "%s @ %s(...)", RENDERERNAME, __FUNCTION__);
//    return g_egl_func.eglMakeCurrent(dpy, draw, read, ctx);
////    return EGL_TRUE;
//}
