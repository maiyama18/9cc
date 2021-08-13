#include "9cc.h"

static bool at_end() {
    return token->kind == TK_EOF;
}

// If the next token is expected reserved, return true and comsume a token.
// Otherwise return false.
static bool consume_reserved(char* op) {
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

static LVar* locals;

static LVar* init_locals() {
    LVar* head = calloc(1, sizeof(LVar));
    head->next = NULL;
    head->name = "";
    head->len = 0;
    head->offset = 0;
    locals = head;
}

static LVar* find_lvar(Token* tok) {
    for (LVar* lvar = locals; lvar; lvar = lvar->next) {
        if (lvar->len == tok->len && !memcmp(lvar->name, tok->str, lvar->len)) {
            return lvar;
        }
    }
    return NULL;
}

static Token* consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    Token* cur = token;
    token = token->next;
    return cur;
}

// If the next token is expected reserved, comsume a token.
// Otherwise report error.
static void expect_reserved(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op) error_at(token->str, "not %s");
    token = token->next;
}

// If next token is number, return its value and consume a token.
// Otherwise report error.
static int expect_number() {
    if (token->kind != TK_NUM) error_at(token->str, "not a number");
    int val = token->val;
    token = token->next;
    return val;
}

static Node* new_binary_node(NodeKind kind, Node* lhs, Node* rhs) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node* new_num_node(int val) {
    Node* node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// primary = "(" expr ")" | num | ident
static Node* primary() {
    Token* tok = consume_ident();
    if (tok) {
        Node* node = calloc(1, sizeof(Node));
        node->kind = ND_LVAR;

        LVar* lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        }
        else {
            lvar = calloc(1, sizeof(LVar));
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->next = locals;
            lvar->offset = locals->offset + 8;
            locals = lvar;

            node->offset = lvar->offset;
        }
        return node;
    }

    if (consume_reserved("(")) {
        Node* node = expr();
        expect_reserved(')');
        return node;
    }
    return new_num_node(expect_number());
}

// unary = ("-" | "+")? primary
static Node* unary() {
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
static Node* mul() {
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
static Node* add() {
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
static Node* relational() {
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
static Node* equality() {
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

// assign       = equality ("=" assign)?
static Node* assign() {
    Node* node = equality();
    if (consume_reserved("=")) {
        node = new_binary_node(ND_ASSIGN, node, assign());
    }
    return node;
}


// expr       = equality
Node* expr() {
    return assign();
}

// stmt       = expr ";"
static Node* stmt() {
    Node* node = expr();
    expect_reserved(';');
    return node;
}

// program    = stmt*
Node* program() {
    init_locals();

    int i = 0;
    while (!at_end()) {
        stmts[i++] = stmt();
    }
    stmts[i] = NULL;
}