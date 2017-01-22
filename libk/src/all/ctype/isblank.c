#include <ctype.h>
int isblank( int ch ) {
    return ch == 0x20 || ch == 0x09;
}