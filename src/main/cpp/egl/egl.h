//
// Created by Swung 0x48 on 2024/10/8.
//

#ifndef FOLD_CRAFT_LAUNCHER_EGL_H
#define FOLD_CRAFT_LAUNCHER_EGL_H

#include <EGL/egl.h>

extern struct egl_func_t g_egl_func;

#define EGL_NATIVE_FUNCTION_END(type,name,...)                                  \
    LOG_D("Use native function: %s @ %s(...)", RENDERERNAME, __FUNCTION__); \
    LOAD_EGL_FUNC(name);                                 \
    type ret = egl_##name(__VA_ARGS__);                                    \
    return ret;                                                             \
}

#define EGL_NATIVE_FUNCTION_END_NO_RETURN(type,name,...)                        \
    LOG_D("Use native function: %s @ %s(...)", RENDERERNAME, __FUNCTION__); \
    LOAD_EGL_FUNC(name);                                 \
    egl_##name(__VA_ARGS__);                                               \
}

#define EGL_NATIVE_FUNCTION_HEAD(type,name,...)                               \
__attribute__((visibility("default"))) type EGLAPIENTRY name(__VA_ARGS__)  {                                  \

#endif //FOLD_CRAFT_LAUNCHER_EGL_H
