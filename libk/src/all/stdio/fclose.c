#include <stdio.h>
#include <kernel/vfs.h>

int fclose(FILE *stream) {
	if(vn_close(stream, NULL)) {
		return EOF;
	} else {
		return 0;
	}
}