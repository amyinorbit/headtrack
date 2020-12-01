//===--------------------------------------------------------------------------------------------===
// debug.h - Debugging utilities (requires GLIBC/Unix-compliant libc implementation)
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

void cc_print_stack();

#ifdef __cplusplus
} /* extern "C" */
#endif
