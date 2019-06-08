//
// Created by root on 19-6-6.
//

#ifndef ARTHOOK_NATIVE_LOAD_H
#define ARTHOOK_NATIVE_LOAD_H

#include "ArtMethod.h"

void GetBaseOffset(JNIEnv *env, ArtMethodSummary &summary);
void CalculateOtherOffset(JNIEnv *env,ArtMethodSummary summary,ArtMethodOffsets &method_offset);
bool is_zygote();
//搜索偏移，确定大小用，切勿更改名称
void reversed0();
void reversed1();
#endif //ARTHOOK_NATIVE_LOAD_H
