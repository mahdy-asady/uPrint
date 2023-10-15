#include <stdio.h>


void output()
{
    printf("I am the replaced function!\n");
}

int f1() {
    return 1;
}

void f2()
{
    if(f1() == 1)
        uPrint("Sample function f2() true condition call\n");
    else 
        uPrint("Sample function f2() false condition call\n");
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
    return;
}

int main()
{
    printf("I'm a target C program.\n");
    f3();
    f2();
    return 1;
}