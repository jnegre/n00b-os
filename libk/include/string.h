#ifndef _STRING_H
#define _STRING_H 1

#include <stddef.h>

// *******
// Copying
// *******

/* Copy block of memory */
void* memcpy(void* restrict, const void* restrict, size_t);
/* Move block of memory */
void* memmove(void*, const void*, size_t);
/* Copy string */
char *strcpy(char *restrict, const char *restrict);

// **********
// Comparison
// **********

/* Compare two blocks of memory */
int memcmp(const void*, const void*, size_t);
/* Compare two strings */
int strcmp(const char*, const char*);

// *********
// Searching
// *********

/* Locate first occurrence of character in string */
char * strchr ( const char *, int );
/* Split string into tokens */
char *strtok_r(char *str, const char *delim, char **saveptr);

// *****
// Other
// *****

/* Fill block of memory */
void* memset(void*, int, size_t);
/* Get string length */
size_t strlen(const char*);

#endif
