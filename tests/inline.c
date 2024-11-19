#include "cd_hook.h"
#include <assert.h>
#define UNUSED(x) (void)x
#define BUFFER_SIZE 2048

char out[BUFFER_SIZE];
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
    UNUSED(a);
    UNUSED(b);
    return 42;
}

int main(void){
    cd_hook_ctx f1_ctx = {.to_hook = (void*)f1, .hook = (void*)f2};
    f1();
    assert(strcmp(out, "f1") == 0);
    ch_inline(&f1_ctx, true);
    f1();
    assert(strcmp(out, "Hooked by f2\n"));
    ch_inline(&f1_ctx, false);
    f1();
    assert(strcmp(out, "f1") == 0);
    cd_hook_ctx f3_ctx = {.to_hook = (void*)f3, .hook = (void*)f4};
    assert(15 == f3(5, 10));
    ch_inline(&f3_ctx, true);
    assert(42 == f3(5, 10));
    ch_inline(&f3_ctx, false);
    assert(15 == f3(5, 10));
}
