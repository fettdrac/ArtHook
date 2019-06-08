//
// Created by root on 19-6-6.
//

#ifndef ARTHOOK_CXX_HELPER_H
#define ARTHOOK_CXX_HELPER_H

#include <cstdlib>
#include "type_def.h"

template<typename U, typename T>
U ForceCast(T *x) {
    return (U) (uintptr_t) x;
}

template<typename U, typename T>
U ForceCast(T &x) {
    return *(U *) &x;
}

template<typename T>
struct Identity {
    using type = T;
};

template<typename R>
static inline R OffsetOf(uintptr_t ptr, size_t offset) {
    return reinterpret_cast<R>(ptr + offset);
}

template<typename R>
static inline R OffsetOf(intptr_t ptr, size_t offset) {
    return reinterpret_cast<R>(ptr + offset);
}

template<typename R>
static inline R OffsetOf(ptr_t ptr, size_t offset) {
    return (R) (reinterpret_cast<intptr_t>(ptr) + offset);
}

template<typename T>
static inline T MemberOf(ptr_t ptr, size_t offset) {
    return *OffsetOf<T *>(ptr, offset);
}

static inline size_t DistanceOf(ptr_t a, ptr_t b) {
    return static_cast<size_t>(
            abs(reinterpret_cast<intptr_t>(b) - reinterpret_cast<intptr_t>(a))
    );
}

template<typename T>
static inline void AssignOffset(ptr_t ptr, size_t offset, T member) {
    *OffsetOf<T *>(ptr, offset) = member;
}

template<typename T>
static inline offset_t GetOffsetByValue(ptr_t obj, T expected_value){
    for (offset_t offset = 0; offset != sizeof(T) * 24; offset += sizeof(T)) {
        if (MemberOf<T>(obj, offset) == expected_value) {
            return offset;
        }
    }
    return 0;
}
#endif //ARTHOOK_CXX_HELPER_H
