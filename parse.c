#include "9cc.h"

int max(int a, int b) {
    if (a > b) return a;
    return b;
}
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

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
        return NULL;
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('Z' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

bool consume(char *op) {
    if ((token->kind != TK_RESERVED && token->kind != TK_RETURN ) ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

bool peek(char *op) {
    if ((token->kind != TK_RESERVED && token->kind != TK_RETURN ) ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    // token = token->next;  just peek
    return true;
}

Token *consume_ident() {
    if (token->kind != TK_IDENT)
        return NULL;
    Token *tmp = token;
    token = token->next;
    return tmp;
}

void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
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

bool startswitch(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // skip space
        if (isspace(*p)) {
            //printf("=== space ===\n");
            p++;
            continue;
        }
        
        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p, 6);
            //token->kind = TK_RETURN;
            //token->str = p;
            //i++;
            p += 6;
            continue;
        }

        if (startswitch(p, "if")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (startswitch(p, "else")) {
            cur = new_token(TK_RESERVED, cur, p, 4);
            p += 4;
            continue;
        }

        if (startswitch(p, "while")) {
            cur = new_token(TK_RESERVED, cur, p, 5);
            p += 5;
            continue;
        }

        if (startswitch(p, "==") || startswitch(p, "!=") || startswitch(p, "<=") || startswitch(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        //if (*p == '+'  || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
        if (strchr("+-*/(){}<>=;", *p)) {
            //printf("=== %c ===\n", *p);
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
       
        if ('a' <= *p && *p <= 'z') {
            char *start_p = p;
            while ('a' <= *p && *p <= 'z') {
                p++;
            }
            cur = new_token(TK_IDENT, cur, start_p, p - start_p);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            //printf("=== %d ===\n", cur->val);
            continue;
        }

        error_at(p, "invalid token");
    }

    new_token(TK_EOF, cur, p, 0);
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

//Node *code[100];

// primary    = num | ident | "(" expr ")"
Node *primary() { 
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = locals->offset + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }

    return new_node_num(expect_number());
}

Node *unary() {
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume("<"))
            node = new_node(ND_LT, add(), node);
        else if (consume("<="))
            node = new_node(ND_LE, add(), node);
        else if (consume(">"))
            node = new_node(ND_LT, node, add());
        else if (consume(">="))
            node = new_node(ND_LE, node, add());
        else
            return node;
    }
}

Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

Node *assign() {
    Node *node = equality();
    if (consume("="))
        node = new_node(ND_ASSIGN, node, assign());
    return node;
}

Node *expr() {
    return assign();
}

Node *stmt() {
    Node *node;
/*
program = stmt*
stmt    = expr ";"
        | "{" stmt* "}"
        | "if" "(" expr ")" stmt ("else" stmt)?
        | "return" expr ";"
*/
    //if (consume(TK_RETURN)) {
    if (consume("return")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    } else if (consume("{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        int cnt = 0;
        for (;;) {
            if (consume("}")) return node;
            node->block_list[cnt] = stmt();
            cnt++;
        }
        return node;

    } else if (consume("if")) {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->cond = expr();
        max_control_syntax_cnt++;
        control_syntax_cnt = max_control_syntax_cnt;
        max_control_syntax_cnt = control_syntax_cnt;
        node->control_syntax_cnt = control_syntax_cnt;
        expect(")");
        node->statement = stmt();
        if (peek("else")) {
            node->kind = ND_IFELSE;  // if 'else' is found, overwite former kind.
        } else {
            control_syntax_cnt--;
        }
        return node;
    } else if (consume("else")) {
            node = calloc(1, sizeof(Node));
            node->kind = ND_ELSE;
            node->control_syntax_cnt = control_syntax_cnt;
            node->statement = stmt();
            control_syntax_cnt--;
            return node;
    } else if (consume("while")) {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->cond = expr();
        max_control_syntax_cnt++;
        control_syntax_cnt = max_control_syntax_cnt;
        max_control_syntax_cnt = control_syntax_cnt;
        node->control_syntax_cnt = control_syntax_cnt;
        expect(")");
        node->statement = stmt();
        control_syntax_cnt--;
        return node;
    } else {
        node = expr();
    }
    if (!consume(";"))
        error_at(token->str, "not ';'");
    
    return node;
}

Node *program() {
    int i = 0;
    while (!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}