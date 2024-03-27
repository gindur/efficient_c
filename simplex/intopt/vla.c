#include <stdio.h>
int main(void)
{
    signed int a = 1;
    signed int* p = &a;
    unsigned int* q = (unsigned int*)p;
    *q = 2;
    printf("a = %d\n", a);
    return 0;
}