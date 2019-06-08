//
// Created by root on 19-6-7.
//

#include <jni.h>
#include <unistd.h>
#include <cstring>
#include <dlfcn.h>
#include "test.h"
#include "../base/jni_helper.h"
#include "ArtMethod.h"
#include "../base/log.h"
#include "../xhook/xhook.h"

static ArtMethod method= nullptr;
void my_testOutput(JNIEnv *env,jobject thiz){
    LOGI("JNIEnv address:%p thisObject address:%p\n",env,thiz);
    LOGE("bug已经被修复");
    method.InvokeOriginal(env,thiz, nullptr);
}

void test_art_hook() {
    JNIEnv *env=GetJniEnv();
    jclass test_class=env->FindClass("com/pvdnc/arthook/TestClass");
    jmethodID test_method=env->GetMethodID(test_class,"testOutput","()V");
    method= ArtMethod(test_method);
    method.Hook(reinterpret_cast<ptr_t>(my_testOutput));
}

int (*org_execv)(const char* __path, char* const* __argv)= nullptr;
extern "C" int my_execv(const char* path,char* const* argv){
    LOGI("execv path is:%s\n",path);
    if(strstr(path,"dex2oat")!= nullptr){
        LOGE("attempt to intercept dex2oat\n");
        return -1;
    }
    return org_execv(path,argv);
}

void test_xhook(){
    xhook_register(".*\\.so$", "execv", reinterpret_cast<void *>(my_execv),
                   reinterpret_cast<void **>(&org_execv));
    xhook_refresh(0);//立即进行hook
    LOGI("org_execv address:%p\n",org_execv);
    execv("dex2oat test", nullptr);
}
