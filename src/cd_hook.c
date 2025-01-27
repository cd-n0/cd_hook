#include "include/cd_hook.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

/* MASSIVE Credits to https://gist.github.com/dutc/991c14bc20ef5a1249c4 */
static uint8_t jmp_bytes[] = {MOV_ACC, ADDRESS_PADDING, JMP_ACC};

struct cd_hook_ctx_s {
    union {
        uint8_t old_bytes[INLINE_LENGTH];
        uint16_t vmt_index;
    } hook_data;
    void *to_hook;
    void *original;
    void *hook;
    cd_hook_type type;
    bool hooked;
};

static bool _change_adress_write_protection(void *address, const bool allow_write){
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

const char *ch_util_status_to_string (const cd_hook_errors status)
{
    switch (status)
    {
        case CD_HOOK_OK:
            return "OK";
        case CD_HOOK_ERROR_ALREADY_HOOKED:
            return "ERR: Already hooked";
        case CD_HOOK_ERROR_WRONG_HOOK_METHOD:
            return "ERR: Used wrong hooking method";
        case CD_HOOK_ERROR_NOT_HOOKED:
            return "ERR: Not hooked";
        case CD_HOOK_ERROR_MEMORY_PROTECTION:
            return "ERR: Couldn't change memory protection";
        case CD_HOOK_ERROR_UNDEFINED:
        default:
            return "ERR: Undefined";
    }
}

static cd_hook_errors _check_hook(const cd_hook_ctx *ctx, const bool hook, const cd_hook_type type){
    if (ctx->hooked){
        if (ctx->type == type){
            if(hook) return CD_HOOK_ERROR_ALREADY_HOOKED;
        } else return CD_HOOK_ERROR_WRONG_HOOK_METHOD;
    } else {
        if(!hook) return CD_HOOK_ERROR_NOT_HOOKED;
    }
    return CD_HOOK_OK;
}

static cd_hook_errors _ch_inline_internal(cd_hook_ctx *ctx, const bool hook){
    cd_hook_errors err = _check_hook(ctx, hook, CD_HOOK_INLINE);
    if(err != CD_HOOK_OK)
        return err;
    
    if (!_change_adress_write_protection(ctx->original, true))
        return CD_HOOK_ERROR_MEMORY_PROTECTION;

    if(hook){
        memcpy(&jmp_bytes[JUMP_ADDRESS_OFFSET], &ctx->hook, sizeof(void*));
        memcpy(ctx->hook_data.old_bytes, ctx->to_hook, sizeof(jmp_bytes));
        memcpy(ctx->to_hook, &jmp_bytes, sizeof(jmp_bytes));
        ctx->type = CD_HOOK_INLINE;
        ctx->hooked = true;
    }
    else {
        memcpy(ctx->to_hook, ctx->hook_data.old_bytes, sizeof(jmp_bytes));
        ctx->type = CD_HOOK_UNDEFINED;
        ctx->hooked = false;
    }

    /* Restore the memory protection before returning */
    if (!_change_adress_write_protection(ctx->original, false))
        return CD_HOOK_ERROR_MEMORY_PROTECTION;

    return CD_HOOK_OK;
}

static cd_hook_errors _ch_vmt_internal(cd_hook_ctx *ctx, const size_t vmt_index, const bool hook){
    cd_hook_errors err = _check_hook(ctx, hook, CD_HOOK_VMT);
    if(err != CD_HOOK_OK)
        return err;

    if (!_change_adress_write_protection(&(*(uintptr_t**)ctx->to_hook)[ctx->hook_data.vmt_index], true))
        return CD_HOOK_ERROR_MEMORY_PROTECTION;

    if (CD_HOOK_UNDEFINED == ctx->type){
        /* to_hook is address of the class so *(*class + vmt_index)     */
        /* gets us the function             = *(VMTaddress + index)     */
        ctx->hook_data.vmt_index = vmt_index;
        ctx->original = (*(uintptr_t***)ctx->to_hook)[ctx->hook_data.vmt_index];
        /* original is address of the function pointed by the pointer   */
    }

    if (hook){
        (*(uintptr_t**)ctx->to_hook)[ctx->hook_data.vmt_index] = (uintptr_t)ctx->hook;
        ctx->type = CD_HOOK_VMT;
        ctx->hooked = true;
    } else {
        (*(uintptr_t**)ctx->to_hook)[ctx->hook_data.vmt_index] = (uintptr_t)ctx->original;
        ctx->type = CD_HOOK_UNDEFINED;
        ctx->hooked = false;
    }

    if (!_change_adress_write_protection(&(*(uintptr_t**)ctx->to_hook)[ctx->hook_data.vmt_index], false))
        return CD_HOOK_ERROR_MEMORY_PROTECTION;

    return CD_HOOK_OK;
}

cd_hook_errors ch_reinitialize_ctx(cd_hook_ctx *ctx, void *to_hook, void *hook){
    if (ctx->hooked) {
        cd_hook_errors e = ch_unhook(ctx);
        if (CD_HOOK_OK != e) return e;
    }
    ctx->to_hook = to_hook; 
    ctx->original = to_hook;
    ctx->hook = hook;

    return CD_HOOK_OK;
}

cd_hook_ctx *ch_create_ctx(void *to_hook, void *hook){
    cd_hook_ctx *ctx = calloc(1, sizeof(*ctx));
    if (ctx){
        ctx->to_hook = to_hook; 
        ctx->original = to_hook;
        ctx->hook = hook;
    }

    return ctx;
}

cd_hook_errors ch_destroy_ctx(cd_hook_ctx *ctx, const bool unhook){
    if (unhook)
        if (CD_HOOK_OK != ch_unhook(ctx)) return CD_HOOK_ERROR_UNDEFINED;
    free(ctx);

    return CD_HOOK_OK;
}

cd_hook_errors ch_unhook(cd_hook_ctx *ctx){
    if (!ctx->hooked) return CD_HOOK_ERROR_NOT_HOOKED;
    switch(ctx->type){
        case CD_HOOK_INLINE:
            return _ch_inline_internal(ctx, false);
        case CD_HOOK_VMT:
            /* Index doesn't matter when unhooking */
            return _ch_vmt_internal(ctx, 0, false);
        default:
            return CD_HOOK_ERROR_UNDEFINED;
    }
}


cd_hook_errors ch_inline(cd_hook_ctx *ctx){
    return _ch_inline_internal(ctx, true);
}

cd_hook_errors ch_vmt(cd_hook_ctx *ctx, const size_t vmt_index){
    return _ch_vmt_internal(ctx, vmt_index, true);
}
