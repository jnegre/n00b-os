#include <stdio.h>

int fgetc(FILE *stream) {
	// int vn_rdwr(file_handle_t* handlep, enum uio_rw rw, char* buffer, size_t* length, const credentials_t* c)
	char out;
	size_t length = 1;
	if(!vn_rdwr(stream, UIO_READ, &out, &length, NULL)) {
		if(length == 1) {
			return out;
		} else {
			// TODO set eof indicator
			return EOF;
		}
	} else {
		// TODO set the error indicator
		return EOF;
	}
}