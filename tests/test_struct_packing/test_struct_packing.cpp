#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    struct foo6 {
        short s;
        char c;
        int flip:1;
        int nybble:4;
        int septet:7;
    };

    printf("%zu\n", sizeof(struct foo6));
}