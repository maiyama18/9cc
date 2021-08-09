#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "arg missing!\n");
    }

    char *p = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("\n");
    printf("main:\n");
    printf("\tmov rax, %ld\n", strtol(p, &p, 10));

    while (*p) {
        switch (*p) {
        case '+':
            p++;
            printf("\tadd rax, %ld\n", strtol(p, &p, 10));
            continue;
        case '-':
            p++;
            printf("\tsub rax, %ld\n", strtol(p, &p, 10));
            continue;
        default:
            fprintf(stderr, "invalid char: %c\n", *p);
            exit(1);
        }
    }

    printf("\tret\n");

    return 0;
}
