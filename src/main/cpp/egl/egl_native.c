#include "egl.h"
#include "../includes.h"
#include "loader.h"

#define DEBUG 0

//EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglBindAPI, EGLenum api) EGL_NATIVE_FUNCTION_END(EGLBoolean, eglBindAPI, api)
EGL_NATIVE_FUNCTION_HEAD(EGLint, eglGetError) EGL_NATIVE_FUNCTION_END(EGLint, eglGetError)
EGL_NATIVE_FUNCTION_HEAD(EGLDisplay, eglGetDisplay, EGLNativeDisplayType display_id) EGL_NATIVE_FUNCTION_END(EGLDisplay, eglGetDisplay, display_id)
EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglInitialize, EGLDisplay dpy, EGLint *major, EGLint *minor) EGL_NATIVE_FUNCTION_END(EGLBoolean, eglInitialize, dpy, major, minor)
EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglTerminate, EGLDisplay dpy) EGL_NATIVE_FUNCTION_END(EGLBoolean, eglTerminate, dpy)
EGL_NATIVE_FUNCTION_HEAD(const char *, eglQueryString, EGLDisplay dpy, EGLint name) EGL_NATIVE_FUNCTION_END(const char *, eglQueryString, dpy, name)
EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglGetConfigs, EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) EGL_NATIVE_FUNCTION_END(EGLBoolean, eglGetConfigs, dpy, configs, config_size, num_config)
EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglChooseConfig, EGLDisplay dpy, const EGLint *attrib_list,
			   EGLConfig *configs, EGLint config_size,
			   EGLint *num_config)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglChooseConfig, dpy, attrib_list,
                        configs, config_size,
                        num_config)
EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglGetConfigAttrib, EGLDisplay dpy, EGLConfig config,
			      EGLint attribute, EGLint *value)
				  EGL_NATIVE_FUNCTION_END(EGLBoolean, eglGetConfigAttrib, dpy, config, attribute, value)

EGL_NATIVE_FUNCTION_HEAD(EGLSurface, eglCreateWindowSurface, EGLDisplay dpy, EGLConfig config,
				  EGLNativeWindowType win,
				  const EGLint *attrib_list)
				  EGL_NATIVE_FUNCTION_END(EGLSurface, eglCreateWindowSurface, dpy, config,
					win,
					attrib_list)
EGL_NATIVE_FUNCTION_HEAD(EGLSurface, eglCreatePbufferSurface, EGLDisplay dpy, EGLConfig config,
				   const EGLint *attrib_list)
				   EGL_NATIVE_FUNCTION_END(EGLSurface, eglCreatePbufferSurface, dpy, config, attrib_list)
EGL_NATIVE_FUNCTION_HEAD(EGLSurface, eglCreatePixmapSurface, EGLDisplay dpy, EGLConfig config,
				  EGLNativePixmapType pixmap,
				  const EGLint *attrib_list)
				  EGL_NATIVE_FUNCTION_END(EGLSurface, eglCreatePixmapSurface, dpy, config,
					pixmap,
					attrib_list)

EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglDestroySurface, EGLDisplay dpy, EGLSurface surface)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglDestroySurface, dpy, surface)

EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglQuerySurface, EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglQuerySurface, dpy, surface, attribute, value)

EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglBindAPI, EGLenum api)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglBindAPI, api)

EGL_NATIVE_FUNCTION_HEAD(EGLenum, eglQueryAPI)
EGL_NATIVE_FUNCTION_END(EGLenum, eglQueryAPI)


EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglWaitClient)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglWaitClient)


EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglReleaseThread)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglReleaseThread)


EGL_NATIVE_FUNCTION_HEAD(EGLSurface, eglCreatePbufferFromClientBuffer, 
	      EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer,
	      EGLConfig config, const EGLint *attrib_list)
		  EGL_NATIVE_FUNCTION_END(EGLSurface, eglCreatePbufferFromClientBuffer, 
			dpy, buftype, buffer,
			config, attrib_list)

EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglSurfaceAttrib, EGLDisplay dpy, EGLSurface surface,
			    EGLint attribute, EGLint value)
				EGL_NATIVE_FUNCTION_END(EGLBoolean, eglSurfaceAttrib, dpy, surface,
				attribute, value)
		
EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglBindTexImage, EGLDisplay dpy, EGLSurface surface, EGLint buffer)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglBindTexImage, dpy, surface, buffer)

EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglReleaseTexImage, EGLDisplay dpy, EGLSurface surface, EGLint buffer)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglReleaseTexImage, dpy, surface, buffer)


EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglSwapInterval, EGLDisplay dpy, EGLint interval)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglSwapInterval, dpy, interval)


EGL_NATIVE_FUNCTION_HEAD(EGLContext, eglCreateContext, EGLDisplay dpy, EGLConfig config,
			    EGLContext share_context,
			    const EGLint *attrib_list)
				EGL_NATIVE_FUNCTION_END(EGLContext, eglCreateContext, dpy, config,
				share_context,
				attrib_list)
EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglDestroyContext, EGLDisplay dpy, EGLContext ctx)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglDestroyContext, dpy, ctx)

EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglMakeCurrent, EGLDisplay dpy, EGLSurface draw,
			  EGLSurface read, EGLContext ctx)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglMakeCurrent, dpy, draw, read, ctx)


EGL_NATIVE_FUNCTION_HEAD(EGLContext, eglGetCurrentContext)
EGL_NATIVE_FUNCTION_END(EGLContext, eglGetCurrentContext)

EGL_NATIVE_FUNCTION_HEAD(EGLSurface, eglGetCurrentSurface, EGLint readdraw)
EGL_NATIVE_FUNCTION_END(EGLSurface, eglGetCurrentSurface, readdraw)

EGL_NATIVE_FUNCTION_HEAD(EGLDisplay, eglGetCurrentDisplay)
EGL_NATIVE_FUNCTION_END(EGLDisplay, eglGetCurrentDisplay)

EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglQueryContext, EGLDisplay dpy, EGLContext ctx,
			   EGLint attribute, EGLint *value)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglQueryContext, dpy, ctx,
				attribute, value)

EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglWaitGL)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglWaitGL)


EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglWaitNative, EGLint engine)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglWaitNative, engine)


//EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglSwapBuffers, EGLDisplay dpy, EGLSurface surface)
//EGL_NATIVE_FUNCTION_END(EGLBoolean, eglSwapBuffers, dpy, surface)

EGL_NATIVE_FUNCTION_HEAD(EGLBoolean, eglCopyBuffers, EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
EGL_NATIVE_FUNCTION_END(EGLBoolean, eglCopyBuffers, dpy, surface, target)


EGL_NATIVE_FUNCTION_HEAD(__eglMustCastToProperFunctionPointerType , eglGetProcAddress, const char *procname)
EGL_NATIVE_FUNCTION_END(__eglMustCastToProperFunctionPointerType , eglGetProcAddress, procname)