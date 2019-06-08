//
// Created by root on 19-6-6.
//

#pragma once
#ifndef ARTHOOK_ARTMETHOD_H
#define ARTHOOK_ARTMETHOD_H

#include <jni.h>
#include <cstdlib>
#include "../base/type_def.h"
#include "../base/cxx_helper.h"

static struct ArtMethodSummary final{
    size_t method_size;
    offset_t access_flags_offset;
    size_t entrypoint_filed_size;
    offset_t jni_code_offset;
};

static struct ArtMethodOffsets final {
    size_t method_size_;
    offset_t jni_code_offset_;
    offset_t quick_code_offset_;
    offset_t access_flags_offset_;
    offset_t dex_code_item_offset_offset_;
    offset_t dex_method_index_offset_;
    offset_t method_index_offset_;

    offset_t interpreter_code_offset_;
    offset_t hotness_count_offset_;//7.0
};

struct RuntimeObjects final {
    ptr_t  runtime_;
    ptr_t  heap_;
    ptr_t  thread_list_;
    ptr_t  class_linker_;
    ptr_t  intern_table_;
    ptr_t quick_generic_jni_trampoline_;
};

struct ArtMethodKey{
    ptr_t decl_class;
    u4 access_flags;

    u4 dex_code_item_offset;

    ptr_t quick_code;
    ptr_t interpreter_code;
    ptr_t jni_code;

    jmethodID original_method;
};

class ArtMethod {
public:

    static ArtMethod* GetStaticInstance(){
        static ArtMethod instance=ArtMethod();
        return &instance;
    }

    static void SetArtPrepared(bool prepared);

    ArtMethod(jmethodID methodID);

    ptr_t GetDeclaringClass() {
        return MemberOf<ptr_t>(current_method_id, 0);
    }//GCRoot引用警告，随时可能被回收

    void SetDeclaringClass(ptr_t declaring_class) {
        AssignOffset<ptr_t>(current_method_id, 0, declaring_class);
    }

    u4 GetAccessFlags();

    void SetAccessFlags(u4 access_flags);

    void AddAccessFlags(u4 access_flags) {
        SetAccessFlags(GetAccessFlags() | access_flags);
    }

    void RemoveAccessFlags(u4 access_flags) {
        SetAccessFlags(GetAccessFlags() & ~access_flags);
    }

    bool HasAccessFlags(u4 access_flags) {
        return (access_flags & GetAccessFlags()) != 0;
    }

    u4 GetDexCodeItemOffset();

    void SetDexCodeItemOffset(u4 item_offset);

    ptr_t GetEntryPointFromQuickCompiledCode();

    void SetEntryPointFromQuickCompiledCode(ptr_t entry_point);

    ptr_t GetEntryPointFromInterpreterCode();

    void SetEntryPointFromInterpreterCode(ptr_t entry_point);

    ptr_t GetEntryPointFromJni();

    void SetEntryPointFromJni(ptr_t entry_point);

    void SetHotnessCount(u2 count);

    void Hook(ptr_t replace);

    jobject InvokeOriginal(JNIEnv *env,jobject thiz,jobjectArray params);
protected:
    jmethodID current_method_id;

    void Backup();
    jmethodID Clone();
    void DealGCRoot();
private:
    ArtMethod();
};


#endif //ARTHOOK_ARTMETHOD_H
