#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

//
// tokenize.c
// 

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

void error_at(char* loc, char* fmt, ...);
Token* tokenize(char* p);

extern char* user_input;

//
// parse.c
//

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

Node* expr();

extern Token* token;

//
// codegen.c
//

void gen(Node* node);