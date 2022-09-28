#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "incorrect arg\n");
        return 1;
    }

    char *p = argv[1];

    printf(".globl	_main\n");
    printf("_main:\n");
    printf("    mov w0, #%ld\n",strtol(p, &p, 10));

    while (*p) {
        if (*p == '+') {
            p++;
            printf("    add w0, w0, #%ld\n", strtol(p, &p, 10));
            continue;
        }

        if (*p == '-') {
            p++;
            printf("    sub w0, w0, #%ld\n", strtol(p, &p, 10));
            continue;
        }

        fprintf(stderr, "unexpected charactor: '%c'\n", *p);
        return 1;
    }

    printf("    ret\n");

return 0;
}