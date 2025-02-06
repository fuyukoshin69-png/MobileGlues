//
// Created by hanji on 2025/2/6.
//

#ifndef MOBILEGLUES_FRAMEBUFFER_H
#define MOBILEGLUES_FRAMEBUFFER_H

#include "gl.h"

/*
 * target glBindFramebuffer(GL_DRAW_FRAMEBUFFER) -- fbo(framebuffer) ---------- COLOR_ATTACHMENT0 -glFramebufferTexture2D- texture_x
 *                                                      |-------glDrawBuffers------ ...
 *                                                      ----------------------- COLOR_ATTACHMENTi -glFramebufferTexture2D- texture_y
 */

#ifdef __cplusplus
extern "C" {
#endif


//struct framebuffer_t {
//    GLenum current_target;
//    struct attachment_t *draw_attachment;
//    struct attachment_t *read_attachment;
//};
//
//extern struct framebuffer_t *bound_framebuffer;

GLint getMaxDrawBuffers();

//void rebind_framebuffer(GLenum old_attachment, GLenum target_attachment);

GLAPI GLAPIENTRY void glBindFramebuffer(GLenum target, GLuint framebuffer);

GLAPI GLAPIENTRY void
glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture,
                       GLint level);

GLAPI GLAPIENTRY void glDrawBuffer(GLenum buf);

GLAPI GLAPIENTRY void glDrawBuffers(GLsizei n, const GLenum *bufs);

GLAPI GLAPIENTRY void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers);

#ifdef __cplusplus
}
#endif

#endif //MOBILEGLUES_FRAMEBUFFER_H
