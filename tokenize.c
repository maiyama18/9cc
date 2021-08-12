#include "9cc.h"

static Token* new_token(TokenKind kind, Token* cur, char* str, int len) {
    Token* t = calloc(1, sizeof(Token));
    t->kind = kind;
    t->str = str;
    t->len = len;

    cur->next = t;
    return t;
}

static bool starts_with(char* p, char* q) {
    return memcmp(p, q, strlen(q)) == 0;
}

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

        if (starts_with(p, "==") || starts_with(p, "!=") || starts_with(p, ">=") || starts_with(p, "<=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char* s = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - s;
            continue;
        }

        error_at(p, "failed to tokenize");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}