//
// Created by root on 19-6-7.
//
#include "jni_helper.h"
#include "../elf/elf_tool.h"
#include "log.h"

static JNIEnv* (*get_jnienv_symbol)()= nullptr;
JNIEnv *GetJniEnv() {
    if (get_jnienv_symbol == nullptr) {
        ptr_t libandroid_runtime = DlOpen("libandroid_runtime.so");
        //LOGI("libandroid_runtime handle address:%p\n",libandroid_runtime);
        get_jnienv_symbol = reinterpret_cast<JNIEnv *(*)()>(DlSym(libandroid_runtime,
                                                              "_ZN7android14AndroidRuntime9getJNIEnvEv"));
        //LOGI("get_jnienv_symbol address:%p\n",get_jnienv_symbol);
    }
    if (get_jnienv_symbol != nullptr)return get_jnienv_symbol();
    return nullptr;
}

bool JNIExceptionClear(JNIEnv *env) {
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }
    return false;
}
