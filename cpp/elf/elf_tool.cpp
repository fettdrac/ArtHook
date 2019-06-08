//
// Created by root on 19-6-7.
// 整个elf文件夹内的实际功能代码都是借用whale的，在此对asLody大佬表示感谢
//

#include "elf_tool.h"
#include "process_map.h"

ptr_t DlOpen(const char* name){
    auto range = whale::FindExecuteMemoryRange(name);
    if (!range->IsValid()) {
        return nullptr;
    }
    whale::ElfImage *image = new whale::ElfImage();
    if (!image->Open(range->path_, range->base_)) {
        delete image;
        return nullptr;
    }
    return reinterpret_cast<void *>(image);
}

ptr_t DlSym(ptr_t handle, const char* name){
    whale::ElfImage *image = reinterpret_cast<whale::ElfImage *>(handle);
    return image->FindSymbol<void *>(name);
}
