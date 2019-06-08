//
// Created by root on 19-6-6.
//

#pragma once
#ifndef ARTHOOK_LOG_H
#define ARTHOOK_LOG_H

#include <android/log.h>
#define TAG "ArtHook"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG ,__VA_ARGS__) // 定义LOGI类型
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG ,__VA_ARGS__) // 定义LOGE类型

#endif //ARTHOOK_LOG_H
