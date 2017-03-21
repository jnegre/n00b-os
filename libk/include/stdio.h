#ifndef _STDIO_H
#define _STDIO_H

#include <kernel/vfs.h>

// Macros
#define EOF (-1)
#ifndef NULL
#define NULL 0
#endif

// Types
typedef file_handle_t FILE;
typedef struct fpos fpos_t;

// File access
FILE *fopen(const char *filename, const char *mode);
int fclose(FILE *stream);

// Unformatted input/output
int fgetc(FILE *stream);

// Formatted input/output
int printf(const char *restrict format, ...);

#endif 