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

// error function
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
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
        error("not '%c'", op);
    token = token->next;
}

int expect_number() {
    //printf("expect_number() token->kind : %d\n", token->kind);
    if (token->kind != TK_NUM)
        error("not a number");
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

Token *tokenize(char *p) {
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

        if (*p == '+'  || *p == '-') {
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

        error("couldnt tokenize");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "incorrect arg\n");
        return 1;
    }

    // tokenize
    // make linked list
    token = tokenize(argv[1]);

    printf(".globl	_main\n");
    printf("_main:\n");
    // first token must be a number
    printf("    mov w0, #%d\n", expect_number());

    while (!at_eof()) {
        if (consume('+')) {
            printf("    add w0, w0, #%d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub w0, w0, #%d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}