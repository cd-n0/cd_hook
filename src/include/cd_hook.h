#ifndef CH_HOOK_H
#define CH_HOOK_H

#include <stddef.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

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

typedef enum ch_hook_type_e{
    CH_HOOK_UNDEFINED = 0,
    CH_HOOK_INLINE,
    CH_HOOK_VMT
} ch_hook_type;

typedef enum ch_hook_errors_e{
    CH_HOOK_OK = 0,
    CH_HOOK_ERROR_ALREADY_HOOKED,
    CH_HOOK_ERROR_WRONG_HOOK_METHOD,
    CH_HOOK_ERROR_NOT_HOOKED,
    CH_HOOK_ERROR_MEMORY_PROTECTION,
    CH_HOOK_ERROR_UNDEFINED
} ch_hook_errors;

typedef struct ch_hook_ctx_s ch_hook_ctx;

/* A quick way to call the original without messing up the hook. Since there's
 * no type checking and a way to get the return value, I still recommend the
* manual  method */
#define CH_CALL_ORIGINAL(CTX, FUNCTION_PROTOTYPE, ...) \
    switch((CTX)->type){\
        case CH_HOOK_INLINE:\
            ch_inline(CTX, false);\
            ((FUNCTION_PROTOTYPE)(CTX)->original)(__VA_ARGS__);\
            ch_inline(CTX, true);\
            break;\
        case CH_HOOK_UNDEFINED:\
        default:\
            fprintf(stderr, "Not hooked or set to a non-existent hook method");\
    }

const char *ch_util_status_to_string (const ch_hook_errors status);
ch_hook_ctx *ch_create_ctx(void *to_hook, void *hook);
ch_hook_errors ch_destroy_ctx(ch_hook_ctx *ctx, const bool unhook);
ch_hook_errors ch_reinitialize_ctx(ch_hook_ctx *ctx, void *to_hook, void *hook);
ch_hook_errors ch_unhook(ch_hook_ctx *ctx);
ch_hook_errors ch_inline(ch_hook_ctx *ctx);
ch_hook_errors ch_vmt(ch_hook_ctx *ctx, const size_t vmt_index);

#if defined(__cplusplus)
}
#endif
#endif /* CH_HOOK_H */
