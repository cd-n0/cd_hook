#include "cd_hook.h"
/* MASSIVE Credits to https://gist.github.com/dutc/991c14bc20ef5a1249c4 */
static uint8_t jmp_bytes[] = {MOV_ACC, ADDRESS_PADDING, JMP_ACC};

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

bool ch_inline(cd_hook_ctx *ctx, bool hook){
    if ((ctx->hooked && hook) || (!ctx->hooked && !hook)) return true;
    
    if (!_change_adress_write_protection(ctx, true)) return false;

    if(hook){
        memcpy(&jmp_bytes[JUMP_ADDRESS_OFFSET], &ctx->hook, sizeof(void*));
        memcpy(ctx->old_bytes, ctx->to_hook, sizeof(jmp_bytes));
        memcpy(ctx->to_hook, &jmp_bytes, sizeof(jmp_bytes));
        ctx->type = HOOK_INLINE;
        ctx->hooked = true;
    }
    else {
        memcpy(ctx->to_hook, ctx->old_bytes, sizeof(jmp_bytes));
        ctx->type = HOOK_UNDEFINED;
        ctx->hooked = false;
    }

    /* Restore the memory protection before returning */
    if (!_change_adress_write_protection(ctx, false)) return false;

    return true;
}
