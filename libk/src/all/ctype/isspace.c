#include <ctype.h>
int isspace( int ch ) {
    return (ch>=0x09 && ch<=0x0D) || ch==0x20;
}