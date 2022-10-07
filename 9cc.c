#include "9cc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "incorrect arg\n");
        return 1;
    }

    // tokenize and parse
    user_input = argv[1];
    token = tokenize();
    Node *node = expr();

    printf(".globl	_main\n");
    printf("_main:\n");

    gen(node);


    printf("    mov w0, w8\n");
    pop_rax();
    printf("    ret\n");
    return 0;
}