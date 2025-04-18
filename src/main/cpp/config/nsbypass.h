//
// Created by hanji on 2025/2/12.
//

#ifndef MOBILEGLUES_PLUGIN_NSBYPASS_H
#define MOBILEGLUES_PLUGIN_NSBYPASS_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool linker_ns_load(const char *lib_search_path);

void *linker_ns_dlopen(const char *name, int flag);

void *linker_ns_dlopen_unique(const char *tmpdir, const char *name, int flag);

#ifdef __cplusplus
}
#endif

#endif //MOBILEGLUES_PLUGIN_NSBYPASS_H
