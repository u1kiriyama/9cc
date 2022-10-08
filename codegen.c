#include "9cc.h"

// rax = w8
// rdi = w9
// rbp = fp(=x29)
// rsp = sp

void push(char *reg) {
    printf("    sub sp, sp, #16\n");
    printf("    str %s, [sp]\n", reg);
}
/*
void push_rax() {
    printf("    sub sp, sp, #16\n");
    printf("    str w8, [sp]\n");
}

void push_rdi() {
    printf("    sub sp, sp, #16\n");
    printf("    str w9, [sp]\n");
}

void push_rbp() {
    printf("    sub sp, sp, #16\n");
    printf("    str fp, [sp]\n");
}
*/
void pop(char *reg) {
    printf("    ldr %s, [sp]\n", reg);
    printf("    add sp, sp, #16\n");
}
/*
void pop_rdi() {
    printf("    ldr w9, [sp]\n");
    printf("    add sp, sp, #16\n");
}

void pop_rax() {
    printf("    ldr w8, [sp]\n");
    printf("    add sp, sp, #16\n");
}

void pop_rbp() {
    printf("    ldr fp, [sp]\n");
    printf("    add sp, sp, #16\n");
}
*/
void gen_lval(Node *node) {
    if (node->kind != ND_LVAR)
        printf("left value is not variable");
    
    printf("    mov w8, fp\n");
    printf("    sub w8, w8, %d\n", node->offset);
    //push_rax();
    push(RAX);
}

void gen(Node *node) {
    switch (node->kind) {
    case ND_NUM:
        printf("    mov w8, #%d\n", node->val);
        push(RAX);
        return;
    case ND_LVAR:
        gen_lval(node);
        pop(RAX);
        printf("    mov w8, [w8]\n");
        push(RAX);
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        pop(RDI);
        pop(RAX);
        printf("    mov [w8], w9\n");
        push(RDI);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    pop(RDI);
    pop(RAX);

    switch (node->kind) {
        case ND_ADD:
            printf("    add w8, w8, w9\n");
            break;
        case ND_SUB:
            printf("    sub w8, w8, w9\n");
            break;
        case ND_MUL:
            printf("    mul w8, w8, w9\n");
            break;
        case ND_DIV:
            printf("    sdiv w8, w8, w9\n");
            break;
        case ND_EQ:
            printf("    cmp w9, w8\n");
            printf("    mrs x8, nzcv\n");
            printf("    lsr w8, w8, #30\n");
            break;
        case ND_NE:
            printf("    cmp w9, w8\n");
            printf("    mrs x8, nzcv\n");
            printf("    lsr w8, w8, #30\n");
            printf("    eor W8, W8, #1\n");
        case ND_LT:
            printf("    cmp w9, w8\n");
            printf("    mrs x8, nzcv\n");
            printf("    lsr w8, w8, #31\n");
            break;
        case ND_LE:
            printf("    cmp w9, w8\n");
            printf("    mrs x8, nzcv\n");
            printf("    lsr w8, w8, #30\n");
            printf("    mrs x9, nzcv\n");
            printf("    lsr w9, w9, #31\n");
            printf("    orr w8, w8, w9\n");
            printf("    and w8, w8, #1\n");
    }
    push(RAX);
}