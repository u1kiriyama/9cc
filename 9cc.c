#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "incorrect arg\n");
        return 1;
    }

    control_syntax_depth = 0;
    control_syntax_depth_max = 0;
    // tokenize and parse
    // result is stored in code
    user_input = argv[1];
    token = tokenize();
    locals = calloc(1, sizeof(LVar));
    block_part = -1;
    block_line = 0;
    program();

    printf(".globl	_main\n");
	printf(".p2align	2\n");
    printf("_main:\n");

    // prologue
    printf(";===== prologue begin =====\n");
    push(RBP);
    printf("    mov fp, sp\n");
    printf("    sub sp, sp, 208\n"); // alphabet x 8byte
    printf(";===== prologue end =====\n");

    block_part = -1;
    block_line = 0;
    for (int i = 0; code[i]; i++) {
        gen(code[i]);
        pop(RAX);
    }

    // epilogue
    printf(";===== epilogue begin =====\n");
    printf("    mov x0, x8\n");
    printf("    mov sp, fp\n");
    pop(RBP);
    printf("    ret\n");
    printf(";===== epilogue end =====\n");
    return 0;
}