//
// Created by root on 19-6-8.
//

#include "thread.h"
#include "../base/type_def.h"
#include "../base/jni_helper.h"
#include <jni.h>

#if defined(__aarch64__)
# define __get_tls() ({ void** __val; __asm__("mrs %0, tpidr_el0" : "=r"(__val)); __val; })
#elif defined(__arm__)
# define __get_tls() ({ void** __val; __asm__("mrc p15, 0, %0, c13, c0, 3" : "=r"(__val)); __val; })
#elif defined(__i386__)
# define __get_tls() ({ void** __val; __asm__("movl %%gs:0, %0" : "=r"(__val)); __val; })
#elif defined(__x86_64__)
# define __get_tls() ({ void** __val; __asm__("mov %%fs:0, %0" : "=r"(__val)); __val; })
#else
#error unsupported architecture
#endif

ptr_t GetCurrentThread(){
    JNIEnv *env=GetJniEnv();
    jclass thread_class=env->FindClass("java/lang/Thread");
    jmethodID current_thread_method=env->GetStaticMethodID(thread_class,"currentThread","()Ljava/lang/Thread;");
    jobject thread=env->CallStaticObjectMethod(thread_class,current_thread_method);
    jfieldID native_peer_field=env->GetFieldID(thread_class,"nativePeer","J");
    if(!JNIExceptionClear(env)){//FieldNotFound
        jlong native_peer=env->GetLongField(thread,native_peer_field);
        return reinterpret_cast<ArtThread*>(native_peer);
    }else{//尝试用约定获取当前线程
        return reinterpret_cast<ArtThread *>(__get_tls()[7/*TLS_SLOT_ART_THREAD_SELF*/]);
    }
}
