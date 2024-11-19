#ifndef CD_HOOK_H
#define CD_HOOK_H

#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <dlfcn.h>
#include <stdbool.h>

/*
i386:
    mov eax, 0x01ffffff
    b8       ffff  ff01
    jmp eax
    ff  e0

x86_64:
    mov rax, 0x01ffffffffffffff
    48b8     ffff ffff ffff ff01
    jmp rax
    ff  e0
*/
#ifdef __x86_64__
#define MOV_ACC 0x48, 0xB8
#define JUMP_ADDRESS_OFFSET 2
#define ADDRESS_PADDING 0, 0, 0, 0, 0, 0, 0, 0 
#define INLINE_LENGTH 12
#elif __i386__
#define MOV_ACC 0xB8
#define JUMP_ADDRESS_OFFSET 1
#define ADDRESS_PADDING 0, 0, 0, 0
#define INLINE_LENGTH 7
#else
#error This architecture is not supported
#endif

#define JMP_ACC 0xff, 0xe0

typedef enum cd_hook_type_e{
    CD_HOOK_UNDEFINED = 0,
    CD_HOOK_INLINE,
} cd_hook_type;

typedef enum cd_hook_errors_e{
    CD_HOOK_OK = 0,
    CD_HOOK_ERROR_ALREADY_HOOKED,
    CD_HOOK_ERROR_WRONG_HOOK_METHOD,
    CD_HOOK_ERROR_NOT_HOOKED,
    CD_HOOK_ERROR_MEMORY_PROTECTION,
    CD_HOOK_ERROR_UNDEFINED,
} cd_hook_errors;

typedef struct cd_hook_ctx_s{
    void *to_hook;
    void *hook;
    uint8_t old_bytes[INLINE_LENGTH];
    cd_hook_type type;
    bool hooked;
} cd_hook_ctx;

/* A quick way to call the original without messing up the hook. Since there's
 * no type checking and a way to get the return value, I still recommend the
* manual  method */
#define CH_CALL_ORIGINAL(CTX, FUNCTION_PROTOTYPE, ...) \
    switch((CTX)->type){\
        case CD_HOOK_INLINE:\
            ch_inline(CTX, false);\
            ((FUNCTION_PROTOTYPE)(CTX)->to_hook)(__VA_ARGS__);\
            ch_inline(CTX, true);\
            break;\
        case CD_HOOK_UNDEFINED:\
        default:\
            fprintf(stderr, "Not hooked or set to a non-existent hook method");\
    }
cd_hook_errors ch_inline(cd_hook_ctx *ctx, bool hook);

#endif /* CD_HOOK_H */
