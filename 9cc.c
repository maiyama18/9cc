#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

typedef enum {
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind;
    Token* next;
    int val;
    char* str;
};

// Current token.
Token* token;

char* user_input;

void error_at(char* loc, char* fmt, ...) {
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

Token* new_token(TokenKind kind, Token* cur, char* str) {
    Token* t = calloc(1, sizeof(Token));
    t->kind = kind;
    t->str = str;

    cur->next = t;
    return t;
}

// Tokenine all the input and return the first token.
Token* tokenize(char* p) {
    Token head;
    head.next = NULL;
    Token* cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "failed to tokenize");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

// If the next token is expected reserved, return true and comsume a token.
// Otherwise return false.
bool consume_reserved(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) return false;
    token = token->next;
    return true;
}

// If the next token is expected reserved, comsume a token.
// Otherwise report error.
void expect_reserved(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) error_at(token->str, "not %s");
    token = token->next;
}

// If next token is number, return its value and consume a token.
// Otherwise report error.
int expect_number() {
    if (token->kind != TK_NUM) error_at(token->str, "not a number");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_end() {
    return token->kind == TK_EOF;
}

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
    Node* lhs;
    Node* rhs;
    int val; // used only when kind is ND_NUM
};

Node* new_binary_node(NodeKind kind, Node* lhs, Node* rhs) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node* new_num_node(int val) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node* expr();

// primary = "(" expr ")" | num
Node* primary() {
    if (consume_reserved('(')) {
        Node* node = expr();
        expect_reserved(')');
        return node;
    }
    return new_num_node(expect_number());
}

// mul = primary ("*" primary | "/" primary)*
Node* mul() {
    Node* node = primary();

    for (;;) {
        if (consume_reserved('*')) {
            node = new_binary_node(ND_MUL, node, primary());
        }
        else if (consume_reserved('/')) {
            node = new_binary_node(ND_DIV, node, primary());
        }
        else {
            return node;
        }
    }
}

// expr = mul ("+" mul | "-" mul)*
Node* expr() {
    Node* node = mul();

    for (;;) {
        if (consume_reserved('+')) {
            node = new_binary_node(ND_ADD, node, mul());
        }
        else if (consume_reserved('-')) {
            node = new_binary_node(ND_SUB, node, mul());
        }
        else {
            return node;
        }
    }
}

void gen(Node* node) {
    if (node->kind == ND_NUM) {
        printf("\tpush %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("\tpop rdi\n");
    printf("\tpop rax\n");

    switch (node->kind) {
    case ND_ADD:
        printf("\tadd rax, rdi\n");
        break;
    case ND_SUB:
        printf("\tsub rax, rdi\n");
        break;
    case ND_MUL:
        printf("\timul rax, rdi\n");
        break;
    case ND_DIV:
        printf("\tcqo\n");
        printf("\tidiv rdi\n");
        break;
    default:
        break;
    }

    printf("\tpush rax\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "arg missing!\n");
    }

    user_input = argv[1];
    token = tokenize(user_input);
    Node* node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    gen(node);
    printf("\tpop rax\n");
    printf("\tret\n");

    return 0;
}
