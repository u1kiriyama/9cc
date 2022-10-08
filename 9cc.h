
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RAX "w8"
#define RDI "w9"
#define RBP "fp"

// input program
char *user_input;

// token kind
typedef enum {
    TK_RESERVED, // simbol
    TK_IDENT,    // identifier
    TK_ASSIGN,
    TK_NUM,      // integer token
    TK_EOF,      // EOF token
} TokenKind;
typedef struct Token Token;
// token type
struct Token {
    TokenKind kind; // type of token
    Token *next;    // next token
    int val;        // number(if kind is TK_NUM)
    char *str;      // token string
    int len;        // length of token
};
// watching
Token *token;

typedef struct Node Node;
// AST node
typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_ASSIGN,
    ND_LVAR,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,
    ND_NUM,
} NodeKind;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;      // for kind is ND_NUM
    int offset;   // for kind is ND_LVAR. offse from BP
};

void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int expect_number();
bool at_eof();
bool startswitch(char *p, char *q);
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

// rule
Node *primary();
Node *unary();
Node *mul();
Node *add();
Node *relational();
Node *equality();
Node *assign();
Node *expr();
Node *stmt();
Node *program();
Node *code[100];

void push(char *reg);
void pop(char *reg);
void gen(Node *node);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);