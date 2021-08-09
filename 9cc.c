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

        if (*p == '+' || *p == '-') {
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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "arg missing!\n");
    }

    user_input = argv[1];
    token = tokenize(user_input);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("\n");
    printf("main:\n");

    printf("\tmov rax, %d\n", expect_number());

    // output assembly with consuming succesive tokens with form '+ <NUM>' or '- <NUM>'
    while (!at_end()) {
        if (consume_reserved('+')) {
            printf("\tadd rax, %d\n", expect_number());
            continue;
        }

        expect_reserved('-');
        printf("\tsub rax, %d\n", expect_number());
    }

    printf("\tret\n");

    return 0;
}
