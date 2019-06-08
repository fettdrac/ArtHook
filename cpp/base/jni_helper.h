//
// Created by root on 19-6-7.
//

#pragma once
#ifndef ARTHOOK_JNI_HELPER_H
#define ARTHOOK_JNI_HELPER_H
#include <jni.h>
JNIEnv *GetJniEnv();
bool JNIExceptionClear(JNIEnv *env);
#endif //ARTHOOK_JNI_HELPER_H
