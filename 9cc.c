#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "arg missing!\n");
    }

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("\n");
    printf("main:\n");
    printf("\tmov rax, %d\n", atoi(argv[1]));
    printf("\tret\n");
}
