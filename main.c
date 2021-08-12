#include "9cc.h"

char* user_input;
Token* token;

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
