//
// Created by root on 19-6-8.
//

#include "app_on_create.h"
#include "../ArtMethod.h"
#include "../../base/log.h"
#include "../../base/jni_helper.h"

static ArtMethod method= nullptr;
void my_CallApplicationOnCreate(JNIEnv *env,jobject thiz,jobject thisApp){
    LOGI("invoke hooked callApplicationOnCreate\n");
    jclass app_class=env->GetObjectClass(thisApp);
    jobjectArray params=env->NewObjectArray(1,app_class,thisApp);
    method.InvokeOriginal(env,thiz, params);
}

void hook_app_on_create() {
    JNIEnv *env=GetJniEnv();
    jclass in_class=env->FindClass("android/app/Instrumentation");
    jmethodID jni_method=env->GetMethodID(in_class,"callApplicationOnCreate","(Landroid/app/Application;)V");
    method= ArtMethod(jni_method);
    method.Hook(reinterpret_cast<ptr_t>(my_CallApplicationOnCreate));
    LOGI("hook callApplicationOnCreate finished\n");
}
