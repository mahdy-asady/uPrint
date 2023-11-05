#include <stdio.h>
#include "uPrint.h"


void output(int count, unsigned char *data)
{
    printf("I am the replaced function!\n");
}

int f1() {
    return 1;
}

void f2()
{
    if(f1() == 1)
        uPrint(&output, "Sample function f2() true condition call\n");
    else 
        uPrint(&output, "Sample function f2() false condition call\n");
    return;
}

void f3()
{
    int a;
    a = a + 1;
    printf("a = %d!\n", a);
    if(a > 1)
        f3();
    else 
        f2();

    uPrint(&output, "Sample text %d, %d", a + 10, 20);
    // convert (1, 2, a + 10, 3, 20);
    return;
}

int main()
{
    printf("I'm a target C program.\n");
    f3();
    f2();
    return 1;
}