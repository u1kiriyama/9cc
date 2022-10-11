#include "9cc.h"

// rax = w8
// rdi = w9
// rbp = fp(=x29)
// rsp = sp

void push(char *reg) {
    printf("    sub sp, sp, #16\n");
    printf("    str %s, [sp]\n", reg);
}

void pop(char *reg) {
    printf("    ldr %s, [sp]\n", reg);
    printf("    add sp, sp, #16\n");
}

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR)
        printf("left value is not variable");
    
    printf("    mov x8, fp\n");
    printf("    sub x8, x8, #%d   ;gen_lval\n", node->offset);
    push(RAX);
}

void gen(Node *node) {
    switch (node->kind) {
    case ND_NUM:
        printf("    mov x10, #%d       ;ND_NUM\n", node->val);
        push(IMM);
        return;
    case ND_LVAR:
        gen_lval(node);
        pop(RAX);
        printf("    ldr x8, [x8]      ;ND_LVAR\n");
        push(RAX);
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        pop(RDI);
        pop(RAX);
        printf("    str x9, [x8]      ;ND_ASSIGN\n");
        push(RDI);
        return;
    case ND_RETURN:
        gen(node->lhs);
        pop(RAX);
        printf("    mov x0, x8\n");
        printf("    mov sp, fp\n");
        pop(RBP);
        printf("    ret\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    pop(RDI);
    pop(RAX);

    switch (node->kind) {
        case ND_ADD:
            printf("    add x8, x8, x9\n");
            break;
        case ND_SUB:
            printf("    sub x8, x8, x9\n");
            break;
        case ND_MUL:
            printf("    mul x8, x8, x9\n");
            break;
        case ND_DIV:
            printf("    sdiv x8, x8, x9\n");
            break;
        case ND_EQ:
            printf("    cmp x9, x8\n");
            printf("    mrs x8, nzcv\n");
            printf("    lsr x8, x8, #30\n");
            break;
        case ND_NE:
            printf("    cmp x9, x8\n");
            printf("    mrs x8, nzcv\n");
            printf("    lsr x8, x8, #30\n");
            printf("    eor x8, x8, #1\n");
        case ND_LT:
            printf("    cmp x9, x8\n");
            printf("    mrs x8, nzcv\n");
            printf("    lsr x8, x8, #31\n");
            break;
        case ND_LE:
            printf("    cmp x9, x8\n");
            printf("    mrs x8, nzcv\n");
            printf("    lsr x8, x8, #30\n");
            printf("    mrs x9, nzcv\n");
            printf("    lsr x9, x9, #31\n");
            printf("    orr x8, x8, x9\n");
            printf("    and x8, x8, #1\n");
    }
    push(RAX);
}