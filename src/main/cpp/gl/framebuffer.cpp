//
// Created by hanji on 2025/2/6.
//

#include "framebuffer.h"
#include "log.h"
#include <vector>
#include <unordered_map>

#define DEBUG 0

struct attachment_t {
    GLenum textarget;
    GLuint texture;
    GLint level;
};

struct framebuffer_t {
    GLuint id;
    GLenum target;
    std::vector<GLenum> attachments;

    // attachment - texture
    std::unordered_map<GLenum, attachment_t> texture_binding;
    int dirty = 0;
};

struct framebuffer_binding_t {
    GLuint draw_fbo = 0;
    GLuint read_fbo = 0;
};

std::unordered_map<GLuint, framebuffer_t> g_framebuffers;
framebuffer_binding_t g_fbo_bindings;

GLint MAX_DRAW_BUFFERS = 0;

GLint getMaxDrawBuffers() {
    if (!MAX_DRAW_BUFFERS) {
        LOAD_GLES(glGetIntegerv, void, GLenum pname, GLint *params)
        gles_glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MAX_DRAW_BUFFERS);
    }
    return MAX_DRAW_BUFFERS;
}

std::vector<GLenum> new_attachments;
void es_rebind_framebuffer() {
    LOAD_GLES(glBindFramebuffer, void, GLenum target, GLuint framebuffer)
    LOAD_GLES(glFramebufferTexture2D, void, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
    LOAD_GLES(glDrawBuffers, void, GLsizei n, const GLenum *bufs)

    // Rebuild all fbos to ES
    for (auto& [fboid, fbo]: g_framebuffers) {
        if (!fbo.dirty || fbo.target != GL_DRAW_FRAMEBUFFER || g_fbo_bindings.draw_fbo != fboid)
            continue;

        gles_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboid);

        // WIP: FBO -> texture binding
        for (size_t i = 0; i < fbo.attachments.size(); ++i) {
            GLenum actual_attachment = GL_COLOR_ATTACHMENT0 + i;
            GLenum gl_attachment = fbo.attachments[i];
            auto& att = fbo.texture_binding[gl_attachment];
            gles_glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, actual_attachment, att.textarget, att.texture, att.level);
        }

        if (fbo.attachments.size() == 1 && fbo.attachments[0] == GL_BACK) {
            gles_glDrawBuffers(1, fbo.attachments.data());
        } else {
            // expect having COLOR_ATTACHMENTSi here
            new_attachments.resize(fbo.attachments.size(), GL_NONE);
            for (size_t i = 0; i < fbo.attachments.size(); ++i) {
                if (fbo.attachments[i] == GL_NONE)
                    new_attachments[i] = GL_NONE;
                else
                    new_attachments[i] = GL_COLOR_ATTACHMENT0 + i;
            }
            gles_glDrawBuffers(new_attachments.size(), new_attachments.data());
        }
    }

    // Bind current fbo bindings to ES
    gles_glBindFramebuffer(GL_DRAW_FRAMEBUFFER, g_fbo_bindings.draw_fbo);
    gles_glBindFramebuffer(GL_READ_FRAMEBUFFER, g_fbo_bindings.read_fbo);

    CHECK_GL_ERROR
}

void glBindFramebuffer(GLenum target, GLuint framebuffer) {
    LOG()

    LOG_D("glBindFramebuffer(0x%x, %d)", target, framebuffer)

    LOAD_GLES(glBindFramebuffer, void, GLenum target, GLuint framebuffer)
    gles_glBindFramebuffer(target, framebuffer);

    switch (target) {
        case GL_READ_FRAMEBUFFER:
            g_fbo_bindings.read_fbo = framebuffer;
            break;
        case GL_DRAW_FRAMEBUFFER:
            g_fbo_bindings.draw_fbo = framebuffer;
            break;
        case GL_FRAMEBUFFER:
            g_fbo_bindings.read_fbo = framebuffer;
            g_fbo_bindings.draw_fbo = framebuffer;
            break;
    }

    g_framebuffers[framebuffer] = {
            .id = framebuffer,
            .target = target,
            .attachments = {}
    };

    CHECK_GL_ERROR
}

void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) {
    LOG()

    LOG_D("glFramebufferTexture2D(0x%x, 0x%x, 0x%x, %d, %d)", target, attachment, textarget, texture, level)

    //if (attachment == GL_DEPTH_ATTACHMENT || attachment == GL_STENCIL_ATTACHMENT || attachment == GL_DEPTH_STENCIL_ATTACHMENT) {
        LOAD_GLES(glFramebufferTexture2D, void, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
        gles_glFramebufferTexture2D(target, attachment, textarget, texture, level);
    //    CHECK_GL_ERROR
    //}

    if (target == GL_FRAMEBUFFER)
        target = GL_DRAW_FRAMEBUFFER;

    GLuint fbo = 0;
    switch (target) {
        case GL_DRAW_FRAMEBUFFER:
            fbo = g_fbo_bindings.draw_fbo;
            break;
        case GL_READ_FRAMEBUFFER:
            fbo = g_fbo_bindings.read_fbo;
            break;
    }

    auto& fb = g_framebuffers[fbo];
    auto& att = fb.texture_binding[attachment];
    att.textarget = textarget;
    att.texture = texture;
    att.level = level;
    fb.dirty = 1;

    CHECK_GL_ERROR
}

void glDrawBuffer(GLenum buffer) {
    LOG()

    glDrawBuffers(1, &buffer);
}

void glDrawBuffers(GLsizei n, const GLenum *bufs) {
    LOG()

    LOG_D("glDrawBuffers(%d, %p), [0]=0x%x", n, bufs, n ? bufs[0] : 0)

    auto& fbo = g_framebuffers[g_fbo_bindings.draw_fbo];
    auto& att = fbo.attachments;

    att.clear();
    att.resize(n);
    for (GLsizei i = 0; i < n; ++i) {
        att[i] = bufs[i];
    }
    fbo.dirty = 1;

    es_rebind_framebuffer();
}

void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers) {
    LOG()

    LOG_D("glDeleteFramebuffers(%d, %p)", n, framebuffers)

    for (int i = 0; i < n; ++i) {
        g_framebuffers.erase(framebuffers[i]);
    }

    LOAD_GLES(glDeleteFramebuffers, void, GLsizei n, const GLuint *framebuffers)
    gles_glDeleteFramebuffers(n, framebuffers);

    CHECK_GL_ERROR
}
