#include <jni.h>
#include <string>
#include <dlfcn.h>
#include "base/log.h"
#include "base/type_def.h"
#include "art/native_load.h"
#include "art/hook_module/create_jvm.h"

extern "C" JNIEXPORT jstring JNICALL
Java_com_pvdnc_arthook_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

class static_initializer
{
public:
    static_initializer() {
        LOGI(__FUNCTION__);
        if(!is_zygote())//必须在zygote中执行
            return;
        hook_create_jvm();//进入java世界
    }
};
static static_initializer s;



