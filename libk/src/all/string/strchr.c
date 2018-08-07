#include <string.h>

char * strchr (const char * str, int character) {
	char c = (char)character;
	size_t i = 0;
	do {
		if(str[i] == c) {
			return &str[i];
		}
	} while(str[i++] != 0);
	return NULL;
}