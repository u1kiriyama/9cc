#include "9cc.h"

// rax = w8
// rdi = w9
// rbp = fp(=x29)
// rsp = sp

void push(char *reg) {
    printf("  ;push\n");
    printf("    sub sp, sp, #16\n");
    printf("    str %s, [sp]\n", reg);
}

void pop(char *reg) {
    printf("  ;pop\n");
    printf("    ldr %s, [sp]\n", reg);
    printf("    add sp, sp, #16\n");
}

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR)
        printf("left value is not variable");
    printf(";gen_lval\n");
    printf("    mov x8, fp\n");
    printf("    sub x8, x8, #%d\n", node->offset - 16); // first offset is function name.
    push(RAX);
    printf(";=== gen_lval\n");
}

void gen(Node *node) {
    if (!node) return;
    int cnt =0;
    switch (node->kind) {
    case ND_NUM:
        printf(";ND_NUM\n");
        printf("    mov x10, #%d\n", node->val);
        push(IMM);
        printf(";=== ND_NUM\n");
        return;
    case ND_LVAR:
        printf(";ND_LVAR\n");
        gen_lval(node);
        pop(RAX);
        printf("    ldr x8, [x8]\n");
        push(RAX);
        printf(";=== ND_LVAR\n");
        return;
    case ND_ASSIGN:
        printf(";ND_ASSIGN\n");
        gen_lval(node->lhs);
        gen(node->rhs);
        pop(RDI);
        pop(RAX);
        //printf("    str x9, [x8]\n");
        push(RDI);
        printf(";=== ND_ASSIGN\n");
        return;
    case ND_IF:
        printf(";ND_IF %03d\n", node->control_syntax_cnt);
        //gen(node->lhs);
        gen(node->cond);
        pop(RAX);
        printf("    cmp x8, #0\n");
        printf("    B.EQ .Lend%03d\n", node->control_syntax_cnt);
        gen(node->statement);
        printf("    .Lend%03d:\n", node->control_syntax_cnt);
        return;
    case ND_IFELSE:
        printf(";ND_IFELSE %03d\n", node->control_syntax_cnt);
        gen(node->cond);
        pop(RAX);
        printf("    cmp x8, #0\n");
        printf("    B.EQ .Lelse%03d\n", node->control_syntax_cnt);
        gen(node->statement);
        printf("    B .Lend%03d\n", node->control_syntax_cnt);
        printf("    .Lelse%03d:\n", node->control_syntax_cnt);
        return;
    case ND_ELSE:
        printf(";ND_ELSE %03d\n", node->control_syntax_cnt);
        gen(node->statement);
        printf("    .Lend%03d:\n", node->control_syntax_cnt);
        return;
    case ND_WHILE:
        printf(";ND_WHILE%03d\n", node->control_syntax_cnt);
        printf("    .Lbegin%03d:\n", node->control_syntax_cnt);
        gen(node->cond);
        pop(RAX);
        printf("    cmp x8, #0\n");
        printf("    B.EQ .Lend%03d\n", node->control_syntax_cnt);
        gen(node->statement);
        printf("    B .Lbegin%03d\n", node->control_syntax_cnt);
        printf("    .Lend%03d:\n", node->control_syntax_cnt);
        return;
    case ND_FOR:
        printf(";ND_FOR%03d\n", node->control_syntax_cnt);
        gen(node->init);
        printf("    .Lbegin%03d:\n", node->control_syntax_cnt);
        gen(node->cond);
        pop(RAX);
        printf("    cmp x8, #0\n");
        printf("    B.EQ .Lend%03d\n", node->control_syntax_cnt);
        gen(node->statement);
        gen(node->fin);
        printf("    B .Lbegin%03d\n", node->control_syntax_cnt);
        printf("    .Lend%03d:\n", node->control_syntax_cnt);
        return;
    case ND_BLOCK:
        printf(";ND_BLOCK\n");
        //int cnt = 0;
        while (node->block_list[cnt]) {
            printf(";gen() in ND_BLOCK %d\n", cnt);
            gen(node->block_list[cnt]);
            printf(";=== gen() in ND_BLOCK %d\n", cnt);
            cnt++;
        }
        printf(";=== ND_BLOCK\n");
        return;
    case ND_FUNC:
        printf(";ND_FUNC\n");
        printf("    stp fp, lr, [sp, #-16]!\n");
	    printf("    mov fp, sp\n");
        while (node->block_list[cnt]) {
            printf("    mov x%d, #%d\n", cnt,  node->block_list[cnt]->val);
            cnt++;
        }
        printf("    bl _%s\n", node->funcname);
        printf("    ldp fp, lr, [sp], #16\n");
        push("x0");
        printf(";=== ND_FUNC\n");
        return;
    case ND_RETURN:
        printf(";ND_RETURN\n");
        gen(node->lhs);
        pop(RAX);
        printf("    mov x0, x8 ;in ND_RETURN\n");
        printf("    mov sp, fp ;in ND_RETURN\n");
        pop(RBP);
        printf("    ret ;in ND_RETURN\n");
        printf(";=== ND_RETURN\n");
        return;
    case ND_TOP:
        printf(";ND_TOP\n");
        printf("_%s:\n", node->funcname);
        // prologue
        printf(";===== prologue begin =====\n");
        push(RBP);
        printf("    mov fp, sp\n");
        //printf("    sub sp, sp, 208\n"); // alphabet x 8byte
        while (node->block_list[cnt]) {
            //printf("    mov x%d, #%d\n", cnt,  node->block_list[cnt]->val);
            //gen(node->block_list[cnt]);
            //printf("    push x%d\n", cnt);
            //push(strcat("x", "1"));
            printf("  ;push(not function)\n");
            printf("    sub sp, sp, #16\n");
            printf("    str x%d, [sp]\n", cnt);
            cnt++;
        }
        printf(";===== prologue end =====\n");
        gen(node->statement);
        printf(";=== ND_TOP\n");
        return;
    case ND_ADDR:
        printf(";ND_ADDR\n");
        gen_lval(node->lhs);
        printf(";=== ND_ADDR\n");
        printf(".endADDR:\n");
        return;
    case ND_DEREF:
        printf(";ND_DEREF\n");
        gen(node->lhs);
        //pop(RAX);
        printf("    ldr x8, [sp] ;ND_DEREF\n");
        printf("    ldr x8, [x8] ;ND_DEREF\n");
        push(RAX);
        printf(";=== ND_DEREF\n");
        printf(".endDEREF:\n");
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
            break;
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
            break;
    }
    push(RAX);
}