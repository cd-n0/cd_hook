#include "../src/include/cd_hook.h"
#include <cassert>
#include <cstring>

#define BUFFER_SIZE 2048

static char out[BUFFER_SIZE];

__attribute__((noinline)) void f1(void);
__attribute__((noinline)) void f2(void);

void f1(void){
    strcpy(out, "f1");
}

void f2(void){
    strcpy(out, "Hooked by f2");
}

int main(void){
    /* BEFORE HOOKING */
    f1();
    assert(0 == strcmp(out, "f1"));

    /* HOOKING */
    ch_hook_ctx ctx;
    ch_initialize_ctx(&ctx, (void*)f1, (void*)f2);
    ch_inline(&ctx);
    f1();
    assert(0 == strcmp(out, "Hooked by f2"));

    CH_CALL_ORIGINAL(&ctx, void(*)(void));
    assert(0 == strcmp(out, "f1"));

    f1();
    assert(0 == strcmp(out, "Hooked by f2"));

    return 0;
}
