//
// Created by root on 19-6-6.
//

#include <jni.h>
#include <string>
#include <dlfcn.h>
#include "native_load.h"
#include "ArtMethod.h"
#include "../base/log.h"
#include "../base/cxx_helper.h"
#include "../elf/elf_tool.h"
#include "../elf/process_map.h"
#include "modifiers.h"
#include "api_level.h"
#include "ARTSymbol.h"
#include "test.h"

inline bool HasAccessFlags(ptr_t artMethod,u4 access_flags) {
     u4 current=MemberOf<u4>(artMethod,12);
    return (access_flags & current) != 0;
}

void reversed0(){}
void reversed1(){}

static JNINativeMethod gMethods[]={
        {"reversed0", "()V", reinterpret_cast<void*>(reversed0)},
        {"reversed1", "()V", reinterpret_cast<void*>(reversed1)}};

void GetBaseOffset(JNIEnv *env, ArtMethodSummary &summary){
    LOGI("int value of true is:%d\n",true);
    //获取一个entrypoint指针的大小
    size_t entrypoint_filed_size = (GetAndroidApiLevel() <= ANDROID_LOLLIPOP) ? 8: sizeof(ptr_t);
    //获取两个连续的ArtMethod（这个在AOSP上测试通过，但是不知道在别的地方可不可行
    jclass obj_class=env->FindClass("java/lang/Object");
    jmethodID internalCloneMethod=env->GetMethodID(obj_class,"internalClone","()Ljava/lang/Object;");
    jmethodID cloneMethod=env->GetMethodID(obj_class,"clone","()Ljava/lang/Object;");//524292
    //获取ArtMethod大小
    size_t method_size=DistanceOf(internalCloneMethod,cloneMethod);
    LOGI("ArtMethod size is:%d\n",method_size);//40
    //获取access_flags偏移
    u4 access_flags=kAccPrivate|kAccNative|kAccFastNative;//524546 我也不知道为什么会有fast_native
    offset_t access_flags_offset=GetOffsetByValue(internalCloneMethod,access_flags);
    LOGI("access_flags_offset is:%d\n",access_flags_offset);//12
    //获取jni_code偏移
    ptr_t libart=DlOpen("libart.so");
    //_ZN3artL20Object_internalCloneEP7_JNIEnvP8_jobject就是internalClone的JNI实现
    ptr_t object_clone_symbol=DlSym(libart,"_ZN3artL20Object_internalCloneEP7_JNIEnvP8_jobject");
    LOGI("object_clone_symbol address:%p\n",object_clone_symbol);//0xb3c909f6
    offset_t jni_code_offset=GetOffsetByValue(internalCloneMethod,object_clone_symbol);
    LOGI("jni_code_offset is:%d\n",jni_code_offset);//32
    //给返回结构体赋值
    summary.method_size=method_size;
    summary.access_flags_offset=access_flags_offset;
    summary.entrypoint_filed_size=entrypoint_filed_size;
    summary.jni_code_offset=jni_code_offset;
}

void CalculateOtherOffset(JNIEnv *env,ArtMethodSummary summary,ArtMethodOffsets &method_offset) {
    offset_t access_flags_offset = summary.access_flags_offset;
    offset_t jni_code_offset = summary.jni_code_offset;
    method_offset.method_size_=summary.method_size;
    method_offset.access_flags_offset_ = summary.access_flags_offset;
    //获取入口偏移
    method_offset.jni_code_offset_ = jni_code_offset;
    method_offset.quick_code_offset_ =
            jni_code_offset + summary.entrypoint_filed_size;//AOT/JIT编译好的代码入口
    //加固代码喜欢动这几个字段，跟hook有关的可能也就index清零
    method_offset.dex_code_item_offset_offset_ = access_flags_offset + sizeof(u4) * 1;
    method_offset.dex_method_index_offset_ = access_flags_offset + sizeof(u4) * 2;
    method_offset.method_index_offset_ = access_flags_offset + sizeof(u4) * 3;
    //解释器（主要给Android 7.0 N之前）
    int api_level=GetAndroidApiLevel();
    ArtBridgeList *bridge_list=ARTSymbol::GetBridgeList();
    if (api_level < ANDROID_N&& bridge_list->interpreter_to_compiled_code != nullptr) {
        method_offset.interpreter_code_offset_ = jni_code_offset - summary.entrypoint_filed_size;
    }
    //方法热度
    if (api_level >= ANDROID_N) {
        method_offset.hotness_count_offset_ =method_offset.method_index_offset_ + sizeof(u2);
    }
}

#define DEFAULT_JNI_VERSION JNI_VERSION_1_6

bool is_zygote(){
    size_t process_name_length=0;
    const char* process_name=whale::GetProcessName(getpid()).c_str();
    LOGI("current process name is:%s\n",process_name);
    if(strcmp("zygote", process_name) == 0){
        return true;
    }
    if(strstr(process_name,"app_process")!= nullptr){
        return true;
    }
    return false;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    if(is_zygote())//zygote进程中我们用static_init处理
        return DEFAULT_JNI_VERSION;
    JNIEnv *env= nullptr;
    vm->GetEnv(reinterpret_cast<void **>(&env), DEFAULT_JNI_VERSION);
    if(env== nullptr){
        LOGI("GetEnv failed\n");
        return JNI_ERR;
    }
    LOGI("JNI_OnLoad start\n");
    ArtMethod::SetArtPrepared(true);

    ArtMethodSummary summary;
    GetBaseOffset(env, summary);
    ArtMethodOffsets method_offset;
    CalculateOtherOffset(env,summary,method_offset);
    ArtMethod::GetStaticInstance();
    ARTSymbol::GetBridgeList();
    test_art_hook();
    test_xhook();

    //注册stub method并验证新方法获取的偏移
    jclass bridge_class=env->FindClass("com/pvdnc/arthook/NativeBridge");
    jint reg_method_ret= env->RegisterNatives(bridge_class,gMethods,2);
    LOGI("reg_method_ret is:%d\n",reg_method_ret);
    jmethodID r0_method=env->GetStaticMethodID(bridge_class,"reversed0","()V");
    jmethodID r1_method=env->GetStaticMethodID(bridge_class,"reversed1","()V");
    //ArtMethod::GetBaseOffset(r0_method,r1_method);


    return DEFAULT_JNI_VERSION;
}
