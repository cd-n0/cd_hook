// TODO: Something doesn't work when optimizations are on
#include "cd_hook.h"
#include <assert.h>
#define UNUSED(x) (void)x
#define BUFFER_SIZE 2048

char out[BUFFER_SIZE];

__attribute__((noinline)) void f1(void);
__attribute__((noinline)) void f2(void);
__attribute__((noinline)) int f3(int a, int b);
__attribute__((noinline)) int f4(int a, int b);

void f1(void){
    strcpy(out, "f1");
}

void f2(void){
    strcpy(out, "Hooked by f2");
}

int f3(int a, int b){
    return a + b;
}

int f4(int a, int b){
    return 10 + a + b;
}

int main(void){
    f1();
    assert(0 == strcmp(out, "f1"));

    cd_hook_ctx *f1_ctx = ch_create_ctx((void*)f1, (void*)f2);
    ch_inline(f1_ctx);
    f1();
    assert(0 == strcmp(out, "Hooked by f2"));

    ch_unhook(f1_ctx);
    f1();
    assert(0 == strcmp(out, "f1"));

    assert(15 == f3(5, 10));

    cd_hook_ctx *f3_ctx = ch_create_ctx((void*)f3, (void*)f4);
    ch_inline(f3_ctx);
    assert(42 == f3(12, 20));

    ch_unhook(f3_ctx);
    assert(25 == f3(5, 20));

    return 0;
}
