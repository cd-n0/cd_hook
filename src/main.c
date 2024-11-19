#include "cd_hook.h"

void f1(void){
    printf("f1\n");
}
void f2(void){
    printf("Hooked by f2\n");
}

int f3(int a, int b){
    printf("I'm f3 and I'll return %d\n", a+b);
    return a + b;
}

int f4(int a, int b){
    printf("I'm f4 and I can do whatever I want, and the arguments of \
the original function is a: %d and b: %d\n", a, b);
    return 42;
}

int main(void){
    cd_hook_ctx f1_ctx = {.to_hook = f1, .hook = f2};
    f1();
    ch_inline(&f1_ctx, true);
    f1();
    CH_CALL_ORIGINAL(&f1_ctx, void(*)(void));
    f1();
    ch_inline(&f1_ctx, false);
    f1();
    cd_hook_ctx f3_ctx = {.to_hook = f3, .hook = f4};
    printf("f3 returned: %d\n", f3(5, 10));
    ch_inline(&f3_ctx, true);
    printf("f3 returned: %d\n", f3(5, 10));
    CH_CALL_ORIGINAL(&f3_ctx, int(*)(int a, int b), 20, 100);
    printf("f3 returned: %d\n", f3(5, 10));
    ch_inline(&f3_ctx, false);
    printf("f3 returned: %d\n", f3(5, 10));
}
