//
// Created by root on 19-6-7.
//

#pragma once
#ifndef ARTHOOK_ELF_TOOL_H
#define ARTHOOK_ELF_TOOL_H

#include "elf_image.h"
ptr_t DlOpen(const char* name);
ptr_t DlSym(ptr_t handle, const char* name);
#endif //ARTHOOK_ELF_TOOL_H
