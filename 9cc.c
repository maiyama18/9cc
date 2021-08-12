#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

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
    int len;
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

Token* new_token(TokenKind kind, Token* cur, char* str, int len) {
    Token* t = calloc(1, sizeof(Token));
    t->kind = kind;
    t->str = str;
    t->len = len;

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

        if ((*p == '<' && *(p + 1) == '=') ||
            (*p == '>' && *(p + 1) == '=') ||
            (*p == '=' && *(p + 1) == '=') ||
            (*p == '!' && *(p + 1) == '=')) {
            cur = new_token(TK_RESERVED, cur, p++, 2);
            p++;
            continue;
        }

        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "failed to tokenize");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

// If the next token is expected reserved, return true and comsume a token.
// Otherwise return false.
bool consume_reserved(char* op) {
    if (
        token->kind != TK_RESERVED ||
        strlen(op) != token->len ||
        memcmp(token->str, op, token->len)
        ) {
        return false;
    }
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
    ND_LT,
    ND_LTE,
    ND_EQ,
    ND_NEQ,
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
    if (consume_reserved("(")) {
        Node* node = expr();
        expect_reserved(')');
        return node;
    }
    return new_num_node(expect_number());
}

// unary = ("-" | "+")? primary
Node* unary() {
    if (consume_reserved("+")) {
        return primary();
    }
    else if (consume_reserved("-")) {
        return new_binary_node(ND_SUB, new_num_node(0), primary());
    }
    else {
        return primary();
    }
}

// mul = unary ("*" unary | "/" unary)*
Node* mul() {
    Node* node = unary();

    for (;;) {
        if (consume_reserved("*")) {
            node = new_binary_node(ND_MUL, node, unary());
        }
        else if (consume_reserved("/")) {
            node = new_binary_node(ND_DIV, node, unary());
        }
        else {
            return node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
Node* add() {
    Node* node = mul();

    for (;;) {
        if (consume_reserved("+")) {
            node = new_binary_node(ND_ADD, node, mul());
        }
        else if (consume_reserved("-")) {
            node = new_binary_node(ND_SUB, node, mul());
        }
        else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node* relational() {
    Node* node = add();

    for (;;) {
        if (consume_reserved("<=")) {
            node = new_binary_node(ND_LTE, node, add());
        }
        else if (consume_reserved(">=")) {
            node = new_binary_node(ND_LTE, add(), node);
        }
        else if (consume_reserved("<")) {
            node = new_binary_node(ND_LT, node, add());
        }
        else if (consume_reserved(">")) {
            node = new_binary_node(ND_LT, add(), node);
        }
        else {
            return node;
        }
    }
}

// equality   = relational ("==" relational | "!=" relational)*
Node* equality() {
    Node* node = relational();

    for (;;) {
        if (consume_reserved("==")) {
            node = new_binary_node(ND_EQ, node, relational());
        }
        else if (consume_reserved("!=")) {
            node = new_binary_node(ND_NEQ, node, relational());
        }
        else {
            return node;
        }
    }
}

// expr       = equality
Node* expr() {
    return equality();
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
    case ND_LT:
        printf("\tcmp rax, rdi\n");
        printf("\tsetl al\n");
        printf("\tmovzb rax, al\n");
        break;
    case ND_LTE:
        printf("\tcmp rax, rdi\n");
        printf("\tsetle al\n");
        printf("\tmovzb rax, al\n");
        break;
    case ND_EQ:
        printf("\tcmp rax, rdi\n");
        printf("\tsete al\n");
        printf("\tmovzb rax, al\n");
        break;
    case ND_NEQ:
        printf("\tcmp rax, rdi\n");
        printf("\tsetne al\n");
        printf("\tmovzb rax, al\n");
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
