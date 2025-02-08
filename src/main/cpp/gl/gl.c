//
// Created by Swung0x48 on 2024/10/8.
//

#include "../includes.h"
#include "gl.h"
#include "glcorearb.h"
#include "log.h"
#include "loader.h"
#include "../gles/loader.h"
#include "mg.h"

#define DEBUG 0

extern GLuint draw_fbo;
extern GLuint read_fbo;

GLAPI GLAPIENTRY void glClearDepth(GLclampd depth) {
	LOG();
	glClearDepthf(depth);
    LOAD_GLES_FUNC(glClear)
	gles_glClear(GL_DEPTH_BUFFER_BIT);
}


void glHint(GLenum target, GLenum mode) {
    LOG();
}

extern GLuint watermark_vao;
extern GLuint watermark_vbo;
extern GLuint watermark_program;
extern int init_watermark_res();
extern int watermark_inited;

void glClear(GLbitfield mask) {
    LOG();

    LOAD_GLES_FUNC(glClear)

    gles_glClear(mask);

    if (!watermark_inited && init_watermark_res() != 0) {
        LOG_E("init_watermark_res failed!")
        abort();
    }

    if (draw_fbo == 0) {
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
    }

    CHECK_GL_ERROR
}