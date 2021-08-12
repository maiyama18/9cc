#include "9cc.h"

char* user_input;
Token* token;
Node* stmts[100];

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "arg missing!\n");
    }

    user_input = argv[1];
    token = tokenize(user_input);
    Node* node = program();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // prologue
    printf("\tpush rbp\n");
    printf("\tmov rbp, rsp\n");
    printf("\tsub rsp, %d\n", 8 * 26);

    for (int i = 0; stmts[i]; i++) {
        gen(stmts[i]);
        printf("\tpop rax\n");
    }

    // epilogue
    printf("\tmov rsp, rbp\n");
    printf("\tpop rbp\n");
    printf("\tret\n");

    return 0;
}
