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

//struct framebuffer_t* bound_framebuffer;

GLint MAX_DRAW_BUFFERS = 0;

GLint getMaxDrawBuffers() {
    if (!MAX_DRAW_BUFFERS) {
        LOAD_GLES(glGetIntegerv, void, GLenum pname, GLint *params)
        gles_glGetIntegerv(GL_MAX_DRAW_BUFFERS, &MAX_DRAW_BUFFERS);
    }
    return MAX_DRAW_BUFFERS;
}

void es_rebind_framebuffer() {
    LOAD_GLES(glBindFramebuffer, void, GLenum target, GLuint framebuffer)
    LOAD_GLES(glFramebufferTexture2D, void, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
    LOAD_GLES(glDrawBuffers, void, GLsizei n, const GLenum *bufs)

    // Rebuild all fbos to ES
    for (auto& [fboid, fbo]: g_framebuffers) {
        if (!fbo.dirty)
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
            std::vector<GLenum> new_attachments(fbo.attachments.size(), GL_NONE);
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

//    if (!bound_framebuffer)
//        return;
//
//    struct attachment_t* attach;
//    if (bound_framebuffer->current_target == GL_DRAW_FRAMEBUFFER)
//        attach = bound_framebuffer->draw_attachment;
//    else
//        attach = bound_framebuffer->read_attachment;
//
//    if (!attach)
//        return;
//
//    struct attachment_t attachment = attach[old_attachment - GL_COLOR_ATTACHMENT0];
//
//    LOAD_GLES(glFramebufferTexture2D, void, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
//    gles_glFramebufferTexture2D(bound_framebuffer->current_target, target_attachment, attachment.textarget, attachment.texture, attachment.level);
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
            .attachments = {}
    };

//    if (!bound_framebuffer)
//        bound_framebuffer = malloc(sizeof(struct framebuffer_t));
//
//    switch (target) {
//        case GL_DRAW_FRAMEBUFFER:
//            free(bound_framebuffer->draw_attachment);
//            bound_framebuffer->draw_attachment = malloc(getMaxDrawBuffers() * sizeof(struct attachment_t));
//            break;
//        case GL_READ_FRAMEBUFFER:
//            free(bound_framebuffer->read_attachment);
//            bound_framebuffer->read_attachment = malloc(getMaxDrawBuffers() * sizeof(struct attachment_t));
//            break;
//        case GL_FRAMEBUFFER:
//            free(bound_framebuffer->draw_attachment);
//            bound_framebuffer->draw_attachment = malloc(getMaxDrawBuffers() * sizeof(struct attachment_t));
//            free(bound_framebuffer->read_attachment);
//            bound_framebuffer->read_attachment = malloc(getMaxDrawBuffers() * sizeof(struct attachment_t));
//            break;
//        default:
//            break;
//    }

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

//    if (bound_framebuffer && attachment - GL_COLOR_ATTACHMENT0 <= getMaxDrawBuffers()) {
//        struct attachment_t* attach;
//        if (target == GL_DRAW_FRAMEBUFFER)
//            attach = bound_framebuffer->draw_attachment;
//        else
//            attach = bound_framebuffer->read_attachment;
//
//        if (attach) {
//            attach[attachment - GL_COLOR_ATTACHMENT0].textarget = textarget;
//            attach[attachment - GL_COLOR_ATTACHMENT0].texture = texture;
//            attach[attachment - GL_COLOR_ATTACHMENT0].level = level;
//        }
//
//        bound_framebuffer->current_target = target;
//    }
//
//    LOAD_GLES(glFramebufferTexture2D, void, GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
//    gles_glFramebufferTexture2D(target, attachment, textarget, texture, level);

    CHECK_GL_ERROR
}

void glDrawBuffer(GLenum buffer) {
    LOG()

    glDrawBuffers(1, &buffer);

//    LOAD_GLES(glDrawBuffers, void, GLsizei n, const GLenum *bufs)
//
//    GLint currentFBO;
//    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
//
//    if (currentFBO == 0) { // 默认帧缓冲
//        GLenum buffers[1] = {GL_NONE};
//        switch (buffer) {
//            case GL_FRONT:
//            case GL_BACK:
//            case GL_NONE:
//                buffers[0] = buffer;
//                gles_glDrawBuffers(1, buffers);
//                break;
//            default:
//                // 生成错误：GL_INVALID_ENUM
//                break;
//        }
//    } else { // FBO场景
//        GLint maxAttachments;
//        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxAttachments);
//
//        if (buffer == GL_NONE) {
//            GLenum *buffers = (GLenum *)alloca(maxAttachments * sizeof(GLenum));
//            for (int i = 0; i < maxAttachments; i++) {
//                buffers[i] = GL_NONE;
//            }
//            gles_glDrawBuffers(maxAttachments, buffers);
//        } else if (buffer >= GL_COLOR_ATTACHMENT0 &&
//                   buffer < GL_COLOR_ATTACHMENT0 + maxAttachments) {
//            GLenum *buffers = (GLenum *)alloca(maxAttachments * sizeof(GLenum));
//            for (int i = 0; i < maxAttachments; i++) {
//                buffers[i] = (i == (buffer - GL_COLOR_ATTACHMENT0)) ? buffer : GL_NONE;
//            }
//            gles_glDrawBuffers(maxAttachments, buffers);
//        } else {
//            // 生成错误：GL_INVALID_ENUM
//        }
//    }
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

    // defer calling es functions

//    for (int i = 0; i < n; i++) {
//        if (bufs[i] >= GL_COLOR_ATTACHMENT0 && bufs[i] <= GL_COLOR_ATTACHMENT0 + getMaxDrawBuffers()) {
//            GLenum target_attachment = GL_COLOR_ATTACHMENT0 + i;
//            new_bufs[i] = target_attachment;
//            if (bufs[i] == target_attachment)
//                continue;
//            rebind_framebuffer(bufs[i], target_attachment);
//        } else {
//            new_bufs[i] = bufs[i];
//        }
//    }

    es_rebind_framebuffer();

//    LOAD_GLES(glDrawBuffers, void, GLsizei n, const GLenum *bufs)
//    gles_glDrawBuffers(n, bufs);

//    CHECK_GL_ERROR
}
