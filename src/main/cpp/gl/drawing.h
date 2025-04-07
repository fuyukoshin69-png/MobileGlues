//
// Created by BZLZHH on 2025/1/29.
//

#ifndef MOBILEGLUES_DRAWING_H
#define MOBILEGLUES_DRAWING_H

#include <dlfcn.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <GLES3/gl32.h>
#include "../includes.h"
#include "gl.h"
#include "glcorearb.h"
#include "log.h"
#include "../gles/loader.h"
#include "mg.h"

#ifdef __cplusplus
extern "C" {
#endif

struct draw_elements_indirect_command_t {
    GLuint  count;
    GLuint  instanceCount;
    GLuint  firstIndex;
    GLint   baseVertex;
    GLuint  reservedMustBeZero;
};

GLAPI GLAPIENTRY void glMultiDrawElementsBaseVertex(GLenum mode, GLsizei *counts, GLenum type, const void *const *indices, GLsizei primcount, const GLint *basevertex);

GLAPI GLAPIENTRY void mg_glMultiDrawElementsBaseVertex_indirect(GLenum mode, GLsizei *counts, GLenum type, const void *const *indices, GLsizei primcount, const GLint *basevertex);

GLAPI GLAPIENTRY void mg_glMultiDrawElementsBaseVertex_multiindirect(GLenum mode, GLsizei *counts, GLenum type, const void *const *indices, GLsizei primcount, const GLint *basevertex);

GLAPI GLAPIENTRY void mg_glMultiDrawElementsBaseVertex_basevertex(GLenum mode, GLsizei *counts, GLenum type, const void *const *indices, GLsizei primcount, const GLint *basevertex);

GLAPI GLAPIENTRY void mg_glMultiDrawElementsBaseVertex_drawelements(GLenum mode, GLsizei *counts, GLenum type, const void *const *indices, GLsizei primcount, const GLint *basevertex);

GLAPI GLAPIENTRY void mg_glMultiDrawElements_indirect(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei primcount);

GLAPI GLAPIENTRY void mg_glMultiDrawElements_multiindirect(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei primcount);

GLAPI GLAPIENTRY void mg_glMultiDrawElements_basevertex(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei primcount);

GLAPI GLAPIENTRY void mg_glMultiDrawElements_drawelements(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei primcount);

GLAPI GLAPIENTRY void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei primcount);

GLAPI GLAPIENTRY void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void* indices);

GLAPI GLAPIENTRY void glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);

GLAPI GLAPIENTRY void glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);

GLAPI GLAPIENTRY void glUniform1i(GLint location, GLint v0);

GLAPI GLAPIENTRY void glDrawArrays(GLenum mode, GLint first, GLsizei count);

GLAPI GLAPIENTRY void glMultiDrawArrays(GLenum mode, const GLint* first, const GLsizei* count, GLsizei drawcount);

#ifdef __cplusplus
}
#endif

#endif //MOBILEGLUES_DRAWING_H
