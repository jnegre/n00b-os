#include <stdio.h>

int fgetc(FILE *stream) {
	char out;
	size_t length = 1;
	int error = vn_rdwr(stream, UIO_READ, &out, &length, NULL);
	if(!error) {
		if(length == 1) {
			return out;
		} else {
			stream->eof = 1;
			return EOF;
		}
	} else {
		stream->error = error;
		return EOF;
	}
}