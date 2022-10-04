#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// token kind
typedef enum {
    TK_RESERVED, // simbol
    TK_NUM,      // number token
    TK_EOF,      // EOF token
} TokenKind;

// AST node
typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

typedef struct Token Token;

// token type
struct Token {
    TokenKind kind; // type of token
    Token *next;    // next token
    int val;        // number(if kind is TK_NUM)
    char *str;      // token string
};

// watching
Token *token;

// input program
char *user_input;

// error function
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error_at(token->str, "expected '%c'", op);
    token = token->next;
}

int expect_number() {
    //printf("expect_number() token->kind : %d\n", token->kind);
    if (token->kind != TK_NUM)
        error_at(token->str, "not a number");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;
    /*
    *     next=NULL
    *    head
    *    |
    *  --+------+----------...
    *    |      |\
    *    cur    | cur
    *     next--+  next--
    *     TK_NUM   TK_RESERVED
    *     20       nil
    *     20       '+'
    */

    while (*p) {
        // skip space
        if (isspace(*p)) {
            //printf("=== space ===\n");
            p++;
            continue;
        }

        //if (*p == '+'  || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
        if (strchr("+-*/()", *p)) {
            //printf("=== %c ===\n", *p);
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            //printf("=== %d ===\n", cur->val);
            continue;
        }

        error_at(p, "invalid token");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *expr();
Node *unary();

Node *primary() {
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }
    return new_node_num(expect_number());
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume('*'))
            node = new_node(ND_MUL, node, unary());
        else if (consume('/'))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary() {
    if (consume('+'))
        return primary();
    if (consume('-'))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

Node *expr() {
    Node *node = mul();

    for (;;) {
        if (consume('+'))
            node = new_node(ND_ADD, node, mul());
        else if (consume('-'))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

void push() {
    printf("    mov x10, sp\n");
    printf("    sub sp, sp, #4\n");
    printf("    sub x10, x10, #4\n");
    printf("    str w8, [x10]\n");
}

void pop_rdi() {
    printf("    ldr w9, [x10]\n");
    printf("    add x10, x10, #4\n");
    printf("    add sp, sp, #4\n");
}

void pop_rax() {
    printf("    ldr w8, [x10]\n");
    printf("    add x10, x10, #4\n");
    printf("    add sp, sp, #4\n");
}

void gen(Node *node) {
    if (node->kind == ND_NUM) {
            printf("    mov w8, #%d\n", node->val);
            push();
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    pop_rdi();
    pop_rax();

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
    }
    push();
}

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