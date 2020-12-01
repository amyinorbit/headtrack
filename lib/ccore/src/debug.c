//===--------------------------------------------------------------------------------------------===
// debug.c - Stack trace and other utilitiess
//
// Created by Amy Parent <amy@amyparent.com>
// Copyright (c) 2020 Amy Parent
// Licensed under the MIT License
// =^•.•^=
//===--------------------------------------------------------------------------------------------===
#include <ccore/debug.h>
#include <ccore/math.h>
#include <ccore/log.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_STACK_DEPTH 64

#if WIN32
#include <windows.h>
#include <dbghelp.h>
#else
#include <execinfo.h>
#endif

#if WIN32

void cc_print_stack() {
    HANDLE process = GetCurrentProcess();
    HANDLE thread = GetCurrentThread();

    CONTEXT context;
    memset(&context, 0, sizeof(CONTEXT));
    context.ContextFlags = CONTEXT_FULL;
    RtlCaptureContext(&context);

    SymInitialize(process, NULL, TRUE);

    DWORD image;
    STACKFRAME64 stackframe;
    ZeroMemory(&stackframe, sizeof(STACKFRAME64));

#ifdef _M_IX86
    image = IMAGE_FILE_MACHINE_I386;
    stackframe.AddrPC.Offset = context.Eip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Ebp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Esp;
    stackframe.AddrStack.Mode = AddrModeFlat;
#elif _M_X64
    image = IMAGE_FILE_MACHINE_AMD64;
    stackframe.AddrPC.Offset = context.Rip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.Rsp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.Rsp;
    stackframe.AddrStack.Mode = AddrModeFlat;
#elif _M_IA64
    image = IMAGE_FILE_MACHINE_IA64;
    stackframe.AddrPC.Offset = context.StIIP;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = context.IntSp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrBStore.Offset = context.RsBSP;
    stackframe.AddrBStore.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = context.IntSp;
    stackframe.AddrStack.Mode = AddrModeFlat;
#endif

    #define MAX_BACKTRACE_LINE (MAX_SYM_NAME + 20)
    char line[MAX_BACKTRACE_LINE];

    cc_print("=== stack trace ===\n");
    for (size_t i = 0; i < 25; i++) {
        BOOL result = StackWalk64(
            image, process, thread,
            &stackframe, &context, NULL,
            SymFunctionTableAccess64, SymGetModuleBase64, NULL
        );

        if(!result) break;

        char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
        PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
        symbol->MaxNameLen = MAX_SYM_NAME;

        DWORD64 displacement = 0;
        if (SymFromAddr(process, stackframe.AddrPC.Offset, &displacement, symbol)) {
            snprintf(line, MAX_BACKTRACE_LINE, "%02d: %s", i, symbol->Name);
        } else {
            snprintf(line, MAX_BACKTRACE_LINE, "%02d: <?>\n", i);
        }
        cc_print(line);
    }
    SymCleanup(process);
}

#else

void cc_print_stack() {
    void *addresses[MAX_STACK_DEPTH];
    int size = backtrace(addresses, MAX_STACK_DEPTH);
    char **symbols = backtrace_symbols(addresses, size);
    cc_print("=== stack trace ===\n");

    char line[1024];
    for(int i = 0; i < size; ++i) {
        snprintf(line, 1024, "%02d: %s\n", i, symbols[i]);
        cc_print(line);
    }
    free(symbols);
    cc_print("\n");
}

#endif
