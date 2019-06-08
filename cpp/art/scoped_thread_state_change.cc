#include "scoped_thread_state_change.h"
#include "../base/log.h"
#include "../base/jni_helper.h"
#include "ARTSymbol.h"

namespace whale {
namespace art {

static volatile long kNoGCDaemonsGuard = 0;

jclass ScopedNoGCDaemons::java_lang_Daemons;
jmethodID ScopedNoGCDaemons::java_lang_Daemons_start;
jmethodID ScopedNoGCDaemons::java_lang_Daemons_stop;

void ScopedNoGCDaemons::Load(JNIEnv *env) {
    java_lang_Daemons = reinterpret_cast<jclass>(env->NewGlobalRef(
            env->FindClass("java/lang/Daemons")));
    if (java_lang_Daemons == nullptr) {
        JNIExceptionClear(env);
        LOGE("java/lang/Daemons API is unavailable.");
        return;
    }
    java_lang_Daemons_start = env->GetStaticMethodID(java_lang_Daemons, "start", "()V");
    if (java_lang_Daemons_start == nullptr) {
        JNIExceptionClear(env);
        java_lang_Daemons_start = env->GetStaticMethodID(java_lang_Daemons, "startPostZygoteFork",
                                                         "()V");
    }
    if (java_lang_Daemons_start == nullptr) {
        LOGE("java/lang/Daemons API is available but no start/startPostZygoteFork method.");
        JNIExceptionClear(env);
    }
    java_lang_Daemons_stop = env->GetStaticMethodID(java_lang_Daemons, "stop", "()V");
    JNIExceptionClear(env);
}

ScopedNoGCDaemons::ScopedNoGCDaemons(JNIEnv *env) : env_(env) {
    if (java_lang_Daemons_start != nullptr) {
        if (__sync_sub_and_fetch(&kNoGCDaemonsGuard, 1) <= 0) {
            env_->CallStaticVoidMethod(java_lang_Daemons, java_lang_Daemons_stop);
            JNIExceptionClear(env_);
        }
    }
}


ScopedNoGCDaemons::~ScopedNoGCDaemons() {
    if (java_lang_Daemons_stop != nullptr) {
        if (__sync_add_and_fetch(&kNoGCDaemonsGuard, 1) == 1) {
            env_->CallStaticVoidMethod(java_lang_Daemons, java_lang_Daemons_start);
            JNIExceptionClear(env_);
        }
    }
}

ScopedSuspendAll::ScopedSuspendAll() {
    ManualControl(true);
}

ScopedSuspendAll::~ScopedSuspendAll() {
    ManualControl(false);
}

    void ScopedSuspendAll::ManualControl(bool suspend) {
        ArtSymbolList *symbols = ARTSymbol::GetSymbol();
        bool supported=(symbols->suspend_vm && symbols->resume_vm);
        if (!supported) {
            LOGE("Suspend VM API is unavailable.");
            return;
        }
        if(suspend) {
            symbols->suspend_vm();
        }else{
            symbols->resume_vm();
        }
    }


}  // namespace art
}  // namespace whale

