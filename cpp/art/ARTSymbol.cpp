//
// Created by root on 19-6-7.
//

#include <jni.h>
#include <cstring>
#include "ARTSymbol.h"
#include "../elf/elf_tool.h"
#include "../base/jni_helper.h"
#include "api_level.h"
#include "../base/log.h"
#include "thread.h"

static ArtSymbolList symbol_list;
static ArtBridgeList bridge_list;
static RuntimeObject runtime_obj;

static bool symbol_inited=false;
ArtSymbolList *ARTSymbol::GetSymbol() {
    if(symbol_inited)return &symbol_list;
    ptr_t libart=DlOpen("libart.so");
    //停止/恢复所有java线程
    symbol_list.suspend_vm= reinterpret_cast<void (*)()>(DlSym(libart, "_ZN3art3Dbg9SuspendVMEv"));
    symbol_list.resume_vm= reinterpret_cast<void (*)()>(DlSym(libart, "_ZN3art3Dbg8ResumeVMEv"));
    //Android 7.0 N profile编译政策
    symbol_list.force_process_profiles= reinterpret_cast<void (*)()>(DlSym(libart, "_ZN3art12ProfileSaver20ForceProcessProfilesEv"));
    //ArtMethod.CopyFrom
    int api_level=GetAndroidApiLevel();
    const char* method_copy_from_name= nullptr;
    if (api_level > ANDROID_O) {
        method_copy_from_name="_ZN3art9ArtMethod8CopyFromEPS0_NS_11PointerSizeE";
    } else if (api_level > ANDROID_N) {
        method_copy_from_name="_ZN3art9ArtMethod8CopyFromEPS0_m";
    } else {
        method_copy_from_name="_ZN3art9ArtMethod8CopyFromEPKS0_m";
    }
    LOGI("method_copy_from_name is:%s\n",method_copy_from_name);
    symbol_list.method_copy_from= reinterpret_cast<void (*)(jmethodID, jmethodID, size_t)>(DlSym(libart, method_copy_from_name));
    //Object.Clone
    symbol_list.object_clone= reinterpret_cast<ptr_t (*)(ptr_t, ptr_t)>(DlSym(libart, "_ZN3art6mirror6Object5CloneEPNS_6ThreadE"));
    if(symbol_list.object_clone== nullptr) {
        symbol_list.object_clone_with_class = reinterpret_cast<ptr_t (*)(ptr_t, ptr_t,ptr_t)>(DlSym(libart,
                                                                                       "_ZN3art6mirror6Object5CloneEPNS_6ThreadEPNS0_5ClassE"));
        symbol_list.object_clone_with_size = reinterpret_cast<ptr_t (*)(ptr_t, ptr_t,size_t)>(DlSym(libart,
                                                                                       "_ZN3art6mirror6Object5CloneEPNS_6ThreadEm"));
    }


    symbol_inited=true;
    return &symbol_list;
}

#define POINTER_SIZE sizeof(ptr_t)


void GetAndroidRuntimeAddress(ptr_t libart,ptr_t &quick_to_jni_bridge) {
    int api_level=GetAndroidApiLevel();
    JNIEnv *env=GetJniEnv();
    JavaVM *vm;
    env->GetJavaVM(&vm);
    runtime_obj.vm=vm;
    //计算跳板
    quick_to_jni_bridge = DlSym(libart,"art_quick_generic_jni_trampoline");
    if (quick_to_jni_bridge == nullptr) {
        LOGE("start to search runtime by risky way");
        ptr_t heap = nullptr;
        ptr_t thread_list = nullptr;
        ptr_t class_linker = nullptr;
        ptr_t intern_table = nullptr;
        
        ptr_t runtime = MemberOf<ptr_t>(vm, POINTER_SIZE);
        runtime_obj.runtime=runtime;

        offset_t start = (POINTER_SIZE == 4) ? 200 : 384;
        offset_t end = start + (100 * POINTER_SIZE);
        for (offset_t offset = start; offset != end; offset += POINTER_SIZE) {
            if (MemberOf<ptr_t>(runtime, offset) == vm) {//根据JVM地址来在Runtime结构体中定位
                size_t class_linker_offset =
                        offset - (POINTER_SIZE * 3) - (2 * POINTER_SIZE);
                if (api_level >= ANDROID_O_MR1) {
                    class_linker_offset -= POINTER_SIZE;
                }
                offset_t intern_table_offset = class_linker_offset - POINTER_SIZE;
                offset_t thread_list_Offset = intern_table_offset - POINTER_SIZE;
                offset_t heap_offset = thread_list_Offset - (4 * POINTER_SIZE);
                if (api_level >= ANDROID_M) {
                    heap_offset -= 3 * POINTER_SIZE;
                }
                if (api_level >= ANDROID_N) {
                    heap_offset -= POINTER_SIZE;
                }
                heap = MemberOf<ptr_t>(runtime, heap_offset);
                thread_list = MemberOf<ptr_t>(runtime, thread_list_Offset);
                class_linker = MemberOf<ptr_t>(runtime, class_linker_offset);
                intern_table = MemberOf<ptr_t>(runtime, intern_table_offset);
                break;
            }
        }
        //将runtime关键结构存入结构体
        runtime_obj.heap = heap;
        runtime_obj.thread_list = thread_list;
        runtime_obj.class_linker = class_linker;
        runtime_obj.intern_table = intern_table;

        start = POINTER_SIZE * 25;
        end = start + (100 * POINTER_SIZE);
        for (offset_t offset = start; offset != end; offset += POINTER_SIZE) {
            if (MemberOf<ptr_t>(class_linker, offset) == intern_table) {
                offset_t target_offset =
                        offset + ((api_level >= ANDROID_M) ? 3 : 5) * POINTER_SIZE;
                quick_to_jni_bridge = MemberOf<ptr_t>(class_linker, target_offset);
                break;
            }
        }
    }
    LOGI("quick_to_jni_bridge address:%p\n",quick_to_jni_bridge);
}

static bool bridge_inited=false;
ArtBridgeList *ARTSymbol::GetBridgeList() {
    if(bridge_inited)
        return &bridge_list;
    ptr_t libart=DlOpen("libart.so");
    //解释器到编译代码
    ptr_t interpreter_to_compiled_code= DlSym(libart,"artInterpreterToCompiledCodeBridge");
    bridge_list.interpreter_to_compiled_code=interpreter_to_compiled_code;
    //编译代码到JNI
    ptr_t quick_to_jni_bridge= nullptr;
    GetAndroidRuntimeAddress(libart,quick_to_jni_bridge);
    bridge_list.quick_to_jni_bridge=quick_to_jni_bridge;
    //编译代码到解释器
    ptr_t quick_to_interpreter=DlSym(libart,"art_quick_to_interpreter_bridge");
    bridge_list.quick_to_interpreter=quick_to_interpreter;

    bridge_inited=true;
    return &bridge_list;
}

ptr_t ARTSymbol::CloneObject(ptr_t art_object) {
    ArtSymbolList *symbol_list_p=GetSymbol();
    if (symbol_list_p->object_clone) {
        return symbol_list_p->object_clone(art_object, GetCurrentThread());
    }
    if (symbol_list_p->object_clone_with_class) {
        return symbol_list_p->object_clone_with_class(art_object, GetCurrentThread(), nullptr);
    }
    return symbol_list_p->object_clone_with_size(art_object, GetCurrentThread(), 0);
}
