#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char *m[32];
    printf("before malloc: m[1] = %p\n", m[1]);
    m[1] = malloc(1024);
    printf("after malloc: m[1] = %p\n", m[1]);
    free(m[1]);
    printf("after free: m[1] = %p\n", m[1]);
    return 0;
}