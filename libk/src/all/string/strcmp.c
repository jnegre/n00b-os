#include <string.h>

int strcmp(const char *lhs, const char *rhs) {
	int i=0;
	while(lhs[i] == rhs[i] && lhs[i] != 0) {
		++i;
	}
	return (unsigned char)lhs[i] - (unsigned char)rhs[i];
}