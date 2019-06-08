# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
set(ART_HOOK
        art/ArtMethod.cpp
        art/ARTSymbol.cpp
        art/thread.cpp
        art/scoped_thread_state_change.cc
        art/JitCompiler.cpp
        art/native_load.cpp)
set(HOOK_MODULE
        art/hook_module/create_jvm.cpp
        art/hook_module/app_on_create.cpp)
set(MEM_DEX
        dex/byte_tool.cpp
        dex/mem_dex.cpp)
set(ELF_TOOL
        elf/elf_image.cc
        elf/process_map.cc
        elf/elf_tool.cpp)
set(XHOOK
        xhook/xhook.c
        xhook/xh_core.c
        xhook/xh_elf.c
        xhook/xh_jni.c
        xhook/xh_log.c
        xhook/xh_util.c
        xhook/xh_version.c)

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED

        # Provides a relative path to your source file(s).
        native-lib.cpp
        base/jni_helper.cpp

        ${ELF_TOOL}
        ${XHOOK}
        ${ART_HOOK}
        ${HOOK_MODULE}
        ${MEM_DEX}
        art/test.cpp
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib

        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})