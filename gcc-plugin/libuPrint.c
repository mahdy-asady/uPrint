#include <stdarg.h>
#include <stdio.h>

void uPrintHelper(void (*cb)(int, unsigned char *), unsigned short int id, char *fmt, ...) {
    unsigned char tmp[1024];
    snprintf((char *)tmp, 1024, "Helper Call id: %d\n", id);
    cb(1, tmp);
}
