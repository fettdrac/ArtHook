//
// Created by root on 19-6-7.
//

#ifndef ARTHOOK_ARTSYMBOL_H
#define ARTHOOK_ARTSYMBOL_H

#include "../base/type_def.h"

struct ArtSymbolList final{
    void (*suspend_vm)();
    void (*resume_vm)();

    void (*force_process_profiles)();

    void (*method_copy_from)(jmethodID this_ptr, jmethodID from, size_t num_bytes);

    ptr_t (*object_clone)(ptr_t object_this, ptr_t thread);
    ptr_t (*object_clone_with_class)(ptr_t object_this, ptr_t thread, ptr_t cls);
    ptr_t (*object_clone_with_size)(ptr_t object_this, ptr_t thread, size_t num_bytes);
};
struct ArtBridgeList final{
    ptr_t interpreter_to_compiled_code;//artInterpreterToCompiledCodeBridge
    ptr_t quick_to_jni_bridge;//quick_generic_jni_trampoline_
    ptr_t quick_to_interpreter;//art_quick_to_interpreter_bridge
};

struct RuntimeObject final{
    JavaVM *vm;
    ptr_t runtime;//runtime_

    ptr_t heap;
    ptr_t thread_list;
    ptr_t class_linker;
    ptr_t intern_table;
};
class ARTSymbol {
public:
    static ArtSymbolList* GetSymbol();
    static ArtBridgeList* GetBridgeList();

    static ptr_t CloneObject(ptr_t art_object);
};


#endif //ARTHOOK_ARTSYMBOL_H
