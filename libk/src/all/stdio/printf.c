#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <kernel/tty.h>
#include <kernel/panic.h>

#define LOWER_DIGITS "0123456789abcdef"
#define UPPER_DIGITS "0123456789ABCDEF"
#define MAX_NUMBER_LENGTH 22 //size in the worst case for 64 bits in octal

static int printf_u(const uintmax_t i, const unsigned short base, const bool uppercase) {
	if(base > 16) {
		panic("Base is bigger than 16");
	}

	const char* digits = uppercase ? UPPER_DIGITS : LOWER_DIGITS;
	if(i == 0) {
		terminal_putchar(digits[0]);
		return 1;
	} else {
		// compute
		char reversed[MAX_NUMBER_LENGTH];
		int written = 0;
		uintmax_t remain = i;
		unsigned short d;
		while(remain != 0) {
			if(written > MAX_NUMBER_LENGTH) {
				panic("Buffer too small in printf_u");
			}
			d = remain % base;
			reversed[written] = digits[d];
			remain = remain / base;
			written++;
		}
		// print in reverse
		for(int j = written-1; j >= 0; j--) {
			terminal_putchar(reversed[j]);
		}
		return written;
	}
}

int printf( const char *restrict format, ... ){
	va_list params;
	va_start(params, format);

	int written = 0;

	while(format[0] != '\0') {
		if(format[0] == '%') {
			switch(format[1]) {
				case '%': {
					terminal_putchar('%');
					written ++;
					break;
				}
				case 'c': {
					char c = (char) va_arg(params, int);
					terminal_putchar(c);
					written ++;
					break;
				}
				case 's': {
					const char* str = va_arg(params, const char*);
					size_t len = strlen(str);
					terminal_write(str, len);
					written += len;
					break;
				}
				case 'x':
				case 'X': {
					unsigned int i = va_arg(params, unsigned int);
					written += printf_u(i, 16, format[1] == 'X');
					break;
				}
				case 'o': {
					unsigned int i = va_arg(params, unsigned int);
					written += printf_u(i, 8, true);
					break;
				}
				case 'u': {
					unsigned int i = va_arg(params, unsigned int);
					written += printf_u(i, 10, true);
					break;
				}
				// FIXME non standard
				case 'J': {
					uintmax_t i = va_arg(params, uintmax_t);
					written += printf_u(i, 16, true);
					break;
				}
				case '\0':
					panic("Unexpected end of format string");
				default:
					panic("Unknown format");
			}
			format += 2;
		} else {
			terminal_putchar(format[0]);
			written ++;
			format++;
		}
	}

	va_end(params);
	return written;
}