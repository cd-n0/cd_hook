#include "include/cd_hook.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

/* MASSIVE Credits to https://gist.github.com/dutc/991c14bc20ef5a1249c4 */
static const uint8_t jmp_bytes_format[] = {MOV_ACC, ADDRESS_PADDING, JMP_ACC};

static inline bool _change_adress_write_protection(void *address, const bool allow_write){
    const int pagesize = sysconf(_SC_PAGE_SIZE);
    void *page = address;
    /* https://stackoverflow.com/a/22971450 */
    if (pagesize % 2 == 0)
        page = (void*)((uintptr_t)page & ~(pagesize - 1));
    else
        page = (void*)((uintptr_t)page & ~(pagesize));
    int mprotect_return;
    mprotect_return = allow_write ?
        mprotect(page, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC) :
        mprotect(page, pagesize, PROT_READ | PROT_EXEC);
    if(mprotect_return) {
        perror("mprotect() failed");
        return false;
    }

    return true;
}

const char *ch_error_to_string (const ch_hook_errors error)
{
    switch (error)
    {
        case CH_HOOK_OK:
            return "OK";
        case CH_HOOK_ERROR_ALREADY_HOOKED:
            return "Already hooked";
        case CH_HOOK_ERROR_WRONG_HOOK_METHOD:
            return "Used wrong hooking method";
        case CH_HOOK_ERROR_NOT_HOOKED:
            return "Not hooked";
        case CH_HOOK_ERROR_MEMORY_PROTECTION:
            return "Couldn't change memory protection";
        case CH_HOOK_ERROR_UNDEFINED:
        default:
            return "Undefined";
    }
}

static inline ch_hook_errors _check_hook(const ch_hook_ctx *ctx, const bool hook, const ch_hook_type type){
    if (ctx->hooked){
        if (ctx->type == type){
            if(hook) return CH_HOOK_ERROR_ALREADY_HOOKED;
        } else return CH_HOOK_ERROR_WRONG_HOOK_METHOD;
    } else {
        if(!hook) return CH_HOOK_ERROR_NOT_HOOKED;
    }
    return CH_HOOK_OK;
}

static inline ch_hook_errors _ch_inline_internal(ch_hook_ctx *ctx, const bool hook){
    ch_hook_errors err = _check_hook(ctx, hook, CH_HOOK_INLINE);
    if(err != CH_HOOK_OK)
        return err;
    
    if (!_change_adress_write_protection(ctx->original, true))
        return CH_HOOK_ERROR_MEMORY_PROTECTION;

    /* TODO: There's probably a better way to do this */
    if(hook){
        char jmp_bytes[sizeof(jmp_bytes_format)];
        memcpy(jmp_bytes, jmp_bytes_format, sizeof(jmp_bytes_format));
        memcpy(&jmp_bytes[JUMP_ADDRESS_OFFSET], &ctx->hook, sizeof(void*));
        memcpy(ctx->hook_data.old_bytes, ctx->to_hook, sizeof(jmp_bytes));
        memcpy(ctx->to_hook, &jmp_bytes, sizeof(jmp_bytes));
        ctx->type = CH_HOOK_INLINE;
        ctx->hooked = true;
    }
    else {
        memcpy(ctx->to_hook, ctx->hook_data.old_bytes, sizeof(jmp_bytes_format));
        ctx->type = CH_HOOK_UNDEFINED;
        ctx->hooked = false;
    }

    /* Restore the memory protection before returning */
    if (!_change_adress_write_protection(ctx->original, false))
        return CH_HOOK_ERROR_MEMORY_PROTECTION;

    return CH_HOOK_OK;
}

static inline ch_hook_errors _ch_vmt_internal(ch_hook_ctx *ctx, const size_t vmt_index, const bool hook){
    ch_hook_errors err = _check_hook(ctx, hook, CH_HOOK_VMT);
    if(err != CH_HOOK_OK)
        return err;

    if (!_change_adress_write_protection(&(*(uintptr_t**)ctx->to_hook)[ctx->hook_data.vmt_index], true))
        return CH_HOOK_ERROR_MEMORY_PROTECTION;

    if (CH_HOOK_UNDEFINED == ctx->type){
        /* to_hook is address of the class so *(*class + vmt_index)     */
        /* gets us the function             = *(VMTaddress + index)     */
        ctx->hook_data.vmt_index = vmt_index;
        ctx->original = (*(uintptr_t***)ctx->to_hook)[ctx->hook_data.vmt_index];
        /* original is address of the function pointed by the pointer   */
    }

    if (hook){
        (*(uintptr_t**)ctx->to_hook)[ctx->hook_data.vmt_index] = (uintptr_t)ctx->hook;
        ctx->type = CH_HOOK_VMT;
        ctx->hooked = true;
    } else {
        (*(uintptr_t**)ctx->to_hook)[ctx->hook_data.vmt_index] = (uintptr_t)ctx->original;
        ctx->type = CH_HOOK_UNDEFINED;
        ctx->hooked = false;
    }

    if (!_change_adress_write_protection(&(*(uintptr_t**)ctx->to_hook)[ctx->hook_data.vmt_index], false))
        return CH_HOOK_ERROR_MEMORY_PROTECTION;

    return CH_HOOK_OK;
}

void ch_initialize_ctx(ch_hook_ctx *ctx, void *to_hook, void *hook){
    memset(ctx, 0, sizeof(*ctx));
    if (ctx){
        ctx->to_hook = to_hook; 
        ctx->original = to_hook;
        ctx->hook = hook;
    }
}

ch_hook_errors ch_reinitialize_ctx(ch_hook_ctx *ctx, void *to_hook, void *hook){
    if (ctx->hooked) {
        ch_hook_errors e = ch_unhook(ctx);
        if (CH_HOOK_OK != e) return e;
    }
    ctx->to_hook = to_hook; 
    ctx->original = to_hook;
    ctx->hook = hook;

    return CH_HOOK_OK;
}

ch_hook_ctx *ch_create_ctx(void *to_hook, void *hook){
    ch_hook_ctx *ctx = calloc(1, sizeof(*ctx));
    if (ctx){
        ctx->to_hook = to_hook; 
        ctx->original = to_hook;
        ctx->hook = hook;
    }

    return ctx;
}

ch_hook_errors ch_destroy_ctx(ch_hook_ctx *ctx, const bool unhook){
    if (unhook)
        if (CH_HOOK_OK != ch_unhook(ctx)) return CH_HOOK_ERROR_UNDEFINED;
    free(ctx);

    return CH_HOOK_OK;
}

ch_hook_errors ch_unhook(ch_hook_ctx *ctx){
    if (!ctx->hooked) return CH_HOOK_ERROR_NOT_HOOKED;
    switch(ctx->type){
        case CH_HOOK_INLINE:
            return _ch_inline_internal(ctx, false);
        case CH_HOOK_VMT:
            /* Index doesn't matter when unhooking */
            return _ch_vmt_internal(ctx, 0, false);
        default:
            return CH_HOOK_ERROR_UNDEFINED;
    }
}


ch_hook_errors ch_inline(ch_hook_ctx *ctx){
    return _ch_inline_internal(ctx, true);
}

ch_hook_errors ch_vmt(ch_hook_ctx *ctx, const size_t vmt_index){
    return _ch_vmt_internal(ctx, vmt_index, true);
}
