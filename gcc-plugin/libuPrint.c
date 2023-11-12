#include <stdarg.h>
#include <stdio.h>

void uPrintHelper(void (*cb)(int, unsigned char *), char *fmt, ...) {
    unsigned char tmp[1024];
    snprintf((char *)tmp, 1024, "Helper Call: %s\n", fmt);
    cb(1, tmp);
}