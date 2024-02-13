#include <stdarg.h>
#include <stdio.h>
#include <string.h>


/**
 * @brief Main callback function that will replaced instead of uPrint function in code using plugin.
 * This function will generate array of bytes that should sent to server in any convenient way.
 * 
 * @param cb : A callback function which will send encoded data.
 * @param id : ID of message (Generated with uPrint plugin and stored in database)
 * @param fmt : Format code that stores each data length
 * @param ... : Primary data
 */

void uPrintHelper(void (*cb)(unsigned long, unsigned char *), unsigned short int id, char *fmt, ...) {
    va_list ap;
    char arg_type;
    unsigned long data_length = 1;

    // Calculate required bytes to store data
    va_start (ap, fmt);
    for (int i = 0; (arg_type = fmt[i]) != '\0'; i++) {
        if(arg_type == '0') { // It is a string pointer. Calculate length using strlen
            char *arg = va_arg(ap, char *);
            data_length += strlen(arg) + 1;
        }
        else {
            va_arg(ap, int);
            data_length += arg_type - '0';
        }
    }
    va_end (ap);


    unsigned char data_serialized[data_length];
    *data_serialized = id;
    va_start (ap, fmt);
    unsigned long pos = 1;
    for (int i = 0; (arg_type = fmt[i]) != '\0'; i++) {
        switch (arg_type)
        {
            case '0': // It is a string pointer. Calculate length using strlen
                {
                    char *arg = va_arg(ap, char *);
                    strcpy((char *)(data_serialized + pos), arg);
                    pos += strlen(arg);
                    // Put null terminator
                    *(data_serialized + pos) = 0;
                    pos++;
                }
                break;
            case '4':
                {
                    int arg = va_arg(ap, int);
                    *((int *)(data_serialized + pos)) = arg;
                    pos += arg_type - '0';
                }
                break;
            case '8':
                {
                    long int arg = va_arg(ap, long int);
                    *((long int *)(data_serialized + pos)) = arg;
                    pos += arg_type - '0';
                }
                break;
            default:
                break;
        }
    }
    va_end (ap);

    cb(data_length, data_serialized);
}
