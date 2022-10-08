#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "incorrect arg\n");
        return 1;
    }

    // tokenize and parse
    // result is stored in code
    user_input = argv[1];
    tokenize();
    program();

    printf(".globl	_main\n");
	printf(".p2align	2\n");
    printf("_main:\n");

    // prologue
    push_rbp();
    printf("    mov fp, sp");
    printf("    sub sp, sp, 208\n"); // alphabet x 8byte

    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        pop_rax();
    }

    // epilogue
    printf("    mov w0, w8\n");
    pop_rax();
    printf("    mov sp, fp\n");
    pop_rbp();
    printf("    ret\n");
    return 0;
}