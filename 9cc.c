#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "incorrect arg\n");
        return 1;
    }

    // tokenize and parse
    // result is stored in code
    user_input = argv[1];
    token = tokenize();
    locals = calloc(1, sizeof(LVar));
    program();

    printf(".globl	_main\n");
	printf(".p2align	2\n");
    printf("_main:\n");

    // prologue
    push(RBP);
    printf("    mov fp, sp\n");
    printf("    sub sp, sp, 208\n"); // alphabet x 8byte

    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        pop(RAX);
    }

    // epilogue
    printf("    mov x0, x8\n");
    //pop(RAX);
    printf("    mov sp, fp\n");
    pop(RBP);
    printf("    ret\n");
    return 0;
}