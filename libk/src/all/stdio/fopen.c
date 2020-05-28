#include <goodies.h>
#include <stdio.h>
#include <kernel/vfs.h>

FILE *fopen(const char *filename, UNUSED const char *mode) {
	// int vn_open(file_handle_t** handlepp, const char* path, const int f, const credentials_t* c);
	FILE *file;
	//TODO set mode and credentials
	if(!vn_open(&file, filename, 0, NULL)) {
		return file;
	} else {
		return NULL;
	}
}