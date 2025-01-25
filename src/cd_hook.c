#include "cd_hook.h"
#include <stdlib.h>

/* MASSIVE Credits to https://gist.github.com/dutc/991c14bc20ef5a1249c4 */
static uint8_t jmp_bytes[] = {MOV_ACC, ADDRESS_PADDING, JMP_ACC};

struct cd_hook_ctx_s {
    void *to_hook;
    void *original;
    void *hook;
    cd_hook_type type;
    uint8_t old_bytes[INLINE_LENGTH];
    bool hooked;
};

static bool _change_adress_write_protection(cd_hook_ctx *ctx, bool allow_write){
    int pagesize = sysconf(_SC_PAGE_SIZE);
    void *page = (char*)ctx->to_hook;
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

const char *cm_util_status_to_str (cd_hook_errors status)
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

static cd_hook_errors _check_hook(cd_hook_ctx *ctx, bool hook, cd_hook_type type){
    if (ctx->hooked){
        if (ctx->type == type){
            if(hook) return CD_HOOK_ERROR_ALREADY_HOOKED;
        } else return CD_HOOK_ERROR_WRONG_HOOK_METHOD;
    } else {
        if(!hook) return CD_HOOK_ERROR_NOT_HOOKED;
    }
    return CD_HOOK_OK;
}

static cd_hook_errors _ch_inline_internal(cd_hook_ctx *ctx, bool hook){
    cd_hook_errors err = _check_hook(ctx, hook, CD_HOOK_INLINE);
    if(err != CD_HOOK_OK)
        return err;
    
    if (!_change_adress_write_protection(ctx, true))
        return CD_HOOK_ERROR_MEMORY_PROTECTION;

    if(hook){
        memcpy(&jmp_bytes[JUMP_ADDRESS_OFFSET], &ctx->hook, sizeof(void*));
        memcpy(ctx->old_bytes, ctx->to_hook, sizeof(jmp_bytes));
        memcpy(ctx->to_hook, &jmp_bytes, sizeof(jmp_bytes));
        ctx->type = CD_HOOK_INLINE;
        ctx->hooked = true;
    }
    else {
        memcpy(ctx->to_hook, ctx->old_bytes, sizeof(jmp_bytes));
        ctx->type = CD_HOOK_UNDEFINED;
        ctx->hooked = false;
    }

    /* Restore the memory protection before returning */
    if (!_change_adress_write_protection(ctx, false))
        return CD_HOOK_ERROR_MEMORY_PROTECTION;

    return CD_HOOK_OK;
}

/* Index doesn't matter when unhooking */
static cd_hook_errors _ch_vmt_internal(cd_hook_ctx *ctx, size_t vmt_index, bool hook){
    cd_hook_errors err = _check_hook(ctx, hook, CD_HOOK_VMT);
    if(err != CD_HOOK_OK)
        return err;

    if (ctx->to_hook == ctx->original){
        ctx->to_hook = *(uintptr_t**)ctx->to_hook + vmt_index;
        /*                            vmt address + index */
        /* original is address of the function pointer */
        ctx->original = &(**(uintptr_t**)ctx->to_hook);
    }

    if (!_change_adress_write_protection(ctx, true))
        return CD_HOOK_ERROR_MEMORY_PROTECTION;

    if (hook){
        /* to_hook becomes the address of the function pointer */
        *(uintptr_t*)ctx->to_hook = (uintptr_t)ctx->hook;
        ctx->type = CD_HOOK_VMT;
        ctx->hooked = true;
    } else {
        if (!_change_adress_write_protection(ctx, true))
            return CD_HOOK_ERROR_MEMORY_PROTECTION;
        *(uintptr_t*)ctx->to_hook = (uintptr_t)ctx->original;
        ctx->type = CD_HOOK_UNDEFINED;
        ctx->hooked = false;
    }

    if (!_change_adress_write_protection(ctx, false))
        return CD_HOOK_ERROR_MEMORY_PROTECTION;

    return CD_HOOK_OK;
}

cd_hook_ctx *ch_create_ctx(void *to_hook, void *hook){
    static cd_hook_ctx ctx;
    ctx.to_hook = to_hook; 
    ctx.original = to_hook;
    ctx.hook = hook;
    ctx.type = CD_HOOK_UNDEFINED;
    ctx.hooked = false;

    return &ctx;
}

cd_hook_errors ch_unhook(cd_hook_ctx *ctx){
    if (!ctx->hooked) return CD_HOOK_ERROR_NOT_HOOKED;
    switch(ctx->type){
        case CD_HOOK_INLINE:
            return _ch_inline_internal(ctx, false);
        case CD_HOOK_VMT:
            return _ch_vmt_internal(ctx, false, 0);
        default:
            return CD_HOOK_ERROR_UNDEFINED;
    }
}


cd_hook_errors ch_inline(cd_hook_ctx *ctx){
    return _ch_inline_internal(ctx, true);
};

cd_hook_errors ch_vmt(cd_hook_ctx *ctx, size_t vmt_index){
    return _ch_vmt_internal(ctx, vmt_index, true);
};
