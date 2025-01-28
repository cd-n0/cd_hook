#include "../src/include/cd_hook.h"
#include <cassert>
#include <cstring>

#define BUFFER_SIZE 2048

static char out[BUFFER_SIZE];

__attribute__((noinline)) void f1(void);
__attribute__((noinline)) void f2(void);
__attribute__((noinline)) void f3(void);

void f1(void){
    strcpy(out, "f1");
}

void f2(void){
    strcpy(out, "Hooked by f2");
}

void f3(void){
    strcpy(out, "Hooked by f3");
}

int main(){
    f1();
    assert(0 == strcmp(out, "f1"));

    ch_hook_ctx *ctx = ch_create_ctx((void*)f1, (void*)f2);
    ch_inline(ctx);
    f1();
    assert(0 == strcmp(out, "Hooked by f2"));

    ch_initialize_ctx(ctx, (void*)f1, (void*)f3);
    f1();
    assert(0 == strcmp(out, "f1"));

    ch_inline(ctx);
    f1();
    assert(0 == strcmp(out, "Hooked by f3"));

    ch_destroy_ctx(ctx, true);

    return 0;
}
