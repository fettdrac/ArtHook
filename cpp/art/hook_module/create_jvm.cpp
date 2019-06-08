//
// Created by root on 19-6-8.
//

#include "create_jvm.h"
#include "../../base/log.h"
#include "../ArtMethod.h"
#include "../../xhook/xhook.h"
#include "app_on_create.h"
#include <jni.h>
#include <dlfcn.h>

static jint(JNICALL *old_create_jvm)(JavaVM **p_vm, JNIEnv **p_env, void *vm_args)= nullptr;

extern "C" jint JNICALL JNI_CreateJavaVM_Impl(JavaVM **p_vm, JNIEnv **p_env, void *vm_args) {
    LOGE("invoke hooked JNI_CreateJavaVM\n");
    if (old_create_jvm == nullptr) {
        LOGE("fail to get old_create_jvm\n");
        return JNI_OK;
    }
    jint ret = old_create_jvm(p_vm, p_env, vm_args);
    if (ret == JNI_OK)ArtMethod::SetArtPrepared(true);//通知：art虚拟机已经初始化完毕
    //hook app启动
    hook_app_on_create();
    return ret;
}

void hook_create_jvm(){
    ArtMethod::SetArtPrepared(false);//确保art虚拟机初始化完毕后再进行art hook
    ptr_t create_jvm_symbol = dlsym(RTLD_DEFAULT, "JNI_CreateJavaVM");
    if (create_jvm_symbol != nullptr) {
        LOGI("create_jvm_symbol address:%p\n",create_jvm_symbol);
        xhook_register(".*\\.so$", "JNI_CreateJavaVM", reinterpret_cast<void *>(JNI_CreateJavaVM_Impl),
                       reinterpret_cast<void **>(&old_create_jvm));
        xhook_refresh(0);//立即进行hook
        LOGI("old_create_jvm address:%p\n",old_create_jvm);
    }
}
