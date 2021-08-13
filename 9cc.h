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
    TK_IDENT,
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
    ND_ASSIGN,
    ND_NUM,
    ND_LVAR,
} NodeKind;

typedef struct Node Node;
struct Node {
    NodeKind kind;
    Node* lhs;
    Node* rhs;
    int val; // used only when kind is ND_NUM
    int offset; // offset from base pointer. used only when kind is ND_LVAR
};

typedef struct LVar LVar;
struct LVar {
    LVar* next;
    char* name;
    int len;
    int offset;
};

Node* expr();
Node* program();

extern Token* token;

extern Node* stmts[100];

//
// codegen.c
//

void error(char* fmt, ...);
void gen(Node* node);