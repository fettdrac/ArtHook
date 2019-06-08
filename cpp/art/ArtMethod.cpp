//
// Created by root on 19-6-6.
//

#include <dlfcn.h>
#include "ArtMethod.h"
#include "../base/cxx_helper.h"
#include "../base/log.h"
#include "modifiers.h"
#include "native_load.h"
#include "api_level.h"
#include "../base/jni_helper.h"
#include "ARTSymbol.h"
#include "scoped_thread_state_change.h"
#include <map>

static int api_level=0;//SDK_INT
static bool art_prepared=false;//art状态
static ArtMethodOffsets method_offset;

ArtMethod::ArtMethod() {//静态初始化，坑：这个constructor是会被写进.init.array的，一加载就执行
    if(api_level>0||!art_prepared)return;//保证只初始化一次
    api_level = GetAndroidApiLevel();
    LOGI("API_LEVEL is:%d\n", api_level);
    JNIEnv *env = GetJniEnv();
    ArtMethodSummary temp_summary;
    GetBaseOffset(env, temp_summary);
    CalculateOtherOffset(env,temp_summary,method_offset);
}

ArtMethod::ArtMethod(jmethodID methodID) {
    current_method_id=methodID;
}

inline u4 ArtMethod::GetAccessFlags() {
    return MemberOf<u4>(current_method_id,method_offset.access_flags_offset_);
}

void ArtMethod::SetAccessFlags(u4 access_flags) {
    AssignOffset<u4>(current_method_id, method_offset.access_flags_offset_, access_flags);
}

u4 ArtMethod::GetDexCodeItemOffset() {
    return MemberOf<u4>(current_method_id, method_offset.dex_code_item_offset_offset_);
}

void ArtMethod::SetDexCodeItemOffset(u4 item_offset) {
        AssignOffset<u4>(current_method_id, method_offset.dex_code_item_offset_offset_, item_offset);
}

ptr_t ArtMethod::GetEntryPointFromQuickCompiledCode() {
    return MemberOf<ptr_t>(current_method_id, method_offset.quick_code_offset_);
}

void ArtMethod::SetEntryPointFromQuickCompiledCode(ptr_t entry_point) {
    AssignOffset<ptr_t>(current_method_id, method_offset.quick_code_offset_, entry_point);
}

ptr_t ArtMethod::GetEntryPointFromInterpreterCode() {
    return MemberOf<ptr_t>(current_method_id, method_offset.interpreter_code_offset_);
}

void ArtMethod::SetEntryPointFromInterpreterCode(ptr_t entry_point) {
    AssignOffset<ptr_t>(current_method_id, method_offset.interpreter_code_offset_, entry_point);
}

ptr_t ArtMethod::GetEntryPointFromJni() {
    return MemberOf<ptr_t>(current_method_id, method_offset.jni_code_offset_);
}

void ArtMethod::SetEntryPointFromJni(ptr_t entry_point) {
    AssignOffset<ptr_t>(current_method_id, method_offset.jni_code_offset_, entry_point);
}

void ArtMethod::SetHotnessCount(u2 count) {
    AssignOffset<u2>(current_method_id, method_offset.hotness_count_offset_, count);
}



void ArtMethod::Hook(ptr_t replace) {
    whale::art::ScopedSuspendAll suspendAll;//stop the world，防止hook过程中的方法调用
    Backup();//备份原方法信息
    ArtSymbolList *symbol_list=ARTSymbol::GetSymbol();
    //开始hook操作
    //>=7.0 强行编译method，使入口均指向quick_compiled_entrypoint
    if(symbol_list->force_process_profiles){
        symbol_list->force_process_profiles();
    }
    SetDexCodeItemOffset(0);//清零字节码
    //>=7.0 禁止内联优化
    if (api_level < ANDROID_O_MR1) {
        AddAccessFlags(kAccCompileDontBother_N);
    } else {
        AddAccessFlags(kAccCompileDontBother_O_MR1);
        AddAccessFlags(kAccPreviouslyWarm_O_MR1);
    }
    bool is_pie_abstract=(api_level > ANDROID_P && !HasAccessFlags(kAccAbstract));//Android 9.0 P的热度值有时拿来做抽象方法的imt_index_
    if (is_pie_abstract|| api_level >= ANDROID_N) {
        SetHotnessCount(0);
    }
    //操作access_flags，将方法native化
    //u4 access_flags=GetAccessFlags();
    AddAccessFlags(kAccNative);
    AddAccessFlags(kAccFastNative);
    if (api_level >= ANDROID_P) {//emmm 对Android9.0 P不了解
        RemoveAccessFlags(kAccCriticalNative_P);
    }
    //将入口全部指向JNI
    ArtBridgeList *bridge_list=ARTSymbol::GetBridgeList();
    SetEntryPointFromQuickCompiledCode(bridge_list->quick_to_jni_bridge);
    if (api_level < ANDROID_N//<android 7.0 N 解释器时代
        && bridge_list->interpreter_to_compiled_code != nullptr) {
        SetEntryPointFromInterpreterCode(bridge_list->interpreter_to_compiled_code);
    }
    //设置JNI入口
    SetEntryPointFromJni(replace);
}

static std::map<jmethodID, ArtMethodKey*> hooked_method_map;
void ArtMethod::Backup() {
    ArtMethodKey *method_key=new ArtMethodKey();
    method_key->decl_class=GetDeclaringClass();
    method_key->access_flags=GetAccessFlags();
    method_key->dex_code_item_offset=GetDexCodeItemOffset();
    method_key->interpreter_code=GetEntryPointFromInterpreterCode();
    method_key->quick_code=GetEntryPointFromQuickCompiledCode();
    method_key->jni_code=GetEntryPointFromJni();

    method_key->original_method=Clone();
    hooked_method_map.insert(std::make_pair(current_method_id,method_key));
}

ArtMethodKey* get_method_key(jmethodID method_id){
    auto entry = hooked_method_map.find(method_id);
    if (entry == hooked_method_map.end()) {
        LOGE("fail to find %p in hooked_method_map",method_id);
        return nullptr;
    }
    return entry->second;
}

jobject ArtMethod::InvokeOriginal(JNIEnv *env,jobject thiz, jobjectArray params) {
    //whale::art::ScopedSuspendAll suspendAll;//stop the world，防止暂时过程中的方法调用
    ArtMethodKey *method_key= get_method_key(current_method_id);
    LOGI("method_key address:%p\n",method_key);
    DealGCRoot();
    /*SetAccessFlags(method_key->access_flags);
    SetDexCodeItemOffset(method_key->dex_code_item_offset);
    SetEntryPointFromInterpreterCode(method_key->interpreter_code);
    SetEntryPointFromQuickCompiledCode(method_key->quick_code);
    SetEntryPointFromJni(method_key->jni_code);*/
    //调用备份方法
    jclass method_class=env->FindClass("java/lang/reflect/Method");
    jmethodID invoke_method=env->GetMethodID(method_class,"invoke","(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
    jclass obj_class=env->FindClass("java/lang/Object");
    jobject java_method=env->ToReflectedMethod(obj_class,
            method_key->original_method,
            static_cast<jboolean>(HasAccessFlags(kAccStatic)));//现场构造，省去NewGlobalRef那一套引用管理（性能换稳定）

    jobject ret = env->CallNonvirtualObjectMethod(
            java_method,
            method_class,
            invoke_method,
            thiz,
            params
    );
    //suspendAll.ManualControl(false);
    return ret;//暂时就当unhook了
}

jmethodID ArtMethod::Clone() {
    ArtSymbolList *symbol_list=ARTSymbol::GetSymbol();
    jmethodID jni_clone_method = nullptr;
    if (api_level < ANDROID_M) {
        jni_clone_method =reinterpret_cast<jmethodID>(ARTSymbol::CloneObject(current_method_id));//6.0以下直接当做对象复制
    } else {
        jni_clone_method = reinterpret_cast<jmethodID>(malloc(method_offset.method_size_));
        if (symbol_list->method_copy_from) {
            symbol_list->method_copy_from(jni_clone_method, current_method_id, sizeof(ptr_t));
        } else {//没有方便的MethodCopyFrom，直接memcpy
            memcpy(jni_clone_method, current_method_id, method_offset.method_size_);
        }
    }
    u4 access_flags=GetAccessFlags();
    ArtMethod clone_method = ArtMethod(jni_clone_method);
    bool is_native = HasAccessFlags(kAccNative);
    if (!HasAccessFlags(kAccDirectFlags)) {//固实化虚方法(private 全都是走的direct-call）
        access_flags &= ~(kAccPublic | kAccProtected);
        access_flags |= kAccPrivate;
    }
    access_flags &= ~kAccSynchronized;//拿掉同步标签，没看懂
    //防止inline优化
    if (api_level < ANDROID_O_MR1) {
        access_flags |= kAccCompileDontBother_N;
    } else {
        access_flags |= kAccCompileDontBother_O_MR1;
        access_flags |= kAccPreviouslyWarm_O_MR1;
    }
    if (!is_native) {
        access_flags |= kAccSkipAccessChecks;
    }
    //Android 7.0 N及以上 对热度和profile的处理
    if (api_level >= ANDROID_N) {
        clone_method.SetHotnessCount(0);
        if (!is_native) {
            ptr_t profiling_info = GetEntryPointFromJni();
            if (profiling_info != nullptr) {
                offset_t end = sizeof(u4) * 4;
                for (offset_t offset = 0; offset != end; offset += sizeof(u4)) {
                    if (MemberOf<ptr_t>(profiling_info, offset) == current_method_id) {
                        AssignOffset<ptr_t>(profiling_info, offset, jni_clone_method);
                    }
                }
            }
        }
    }
    ArtBridgeList *bridge_list=ARTSymbol::GetBridgeList();
    if (!is_native && bridge_list->quick_to_interpreter) {//有解释器走解释器模式
        clone_method.SetEntryPointFromQuickCompiledCode(bridge_list->quick_to_interpreter);
    }

    clone_method.SetAccessFlags(access_flags);

    return jni_clone_method;
}

void ArtMethod::DealGCRoot() {
    ArtMethodKey *method_key=get_method_key(current_method_id);
    if(method_key->decl_class==GetDeclaringClass())
        return;
    LOGE("Notice: MovingGC cause the GcRoot References changed.");
    //whale::art::ScopedSuspendAll suspend_all;//在我们修改任何ArtMethod字段前都应该stop the world
    //这里省略了线程锁，如果突然出现段错误之类的就加上
    //重新clone，并且恢复为原方法
    method_key->original_method=Clone();
    ArtMethod origin_method(method_key->original_method);
    //注意，这里没有恢复解释器入口
    origin_method.SetEntryPointFromQuickCompiledCode(method_key->quick_code);
    origin_method.SetEntryPointFromJni(method_key->jni_code);
    origin_method.SetDexCodeItemOffset(method_key->dex_code_item_offset);
    method_key->decl_class = GetDeclaringClass();
}

void ArtMethod::SetArtPrepared(bool prepared) {
    LOGI("switch art_prepared to:%d\n",prepared);
    bool old_art_prepared=art_prepared;//备份art_prepared状态
    art_prepared=prepared;
    if(!old_art_prepared&&prepared){
        ArtMethod();//确保初始化
    }
}