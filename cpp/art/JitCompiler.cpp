//
// Created by root on 19-6-6.
//

#include "JitCompiler.h"
#include "../base/type_def.h"
#include "../base/log.h"
#include <dlfcn.h>

void* (*jitLoad)(bool*) = nullptr;
bool (*jitCompileMethod)(void*, void*, void*, bool) = nullptr;
ptr_t global_jit= nullptr;
JitCompiler::JitCompiler() {
    ptr_t libart_compiler= dlopen("/system/lib/libart-compiler.so",RTLD_LOCAL);
    jitCompileMethod= reinterpret_cast<bool (*)(void *, void *, void *, bool)>
            (dlsym(libart_compiler,"jit_compile_method"));
    LOGI("jit_compile_method_symbol address:%p\n",jitCompileMethod);
    jitLoad= reinterpret_cast<void *(*)(bool *)>(dlsym(libart_compiler, "jit_load"));
    bool debug_info=false;
    global_jit=jitLoad(&debug_info);
    LOGI("global_jit address:%p\n",global_jit);
}
