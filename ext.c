#include <stdio.h>
int foo() {
    printf("foo\n");
    return 0;
}

int calc(int a, int b, int c, int d, int e) {
    printf("%d\n", a*b+c-d*e);
    return 0;
}

int calc2(int a, int b, int c, int d, int e) {
    printf("%d\n", a*b+c-d*e);
    return 0;
}