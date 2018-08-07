#include <string.h>

char *strtok_r(char *str, const char *delim, char **saveptr) {
	if(str != NULL) {
		*saveptr = str;
	}
	//skip initial delim
	while(**saveptr != 0 && strchr(delim, **saveptr)!=NULL) {
		(*saveptr)++;
	}
	char* start = *saveptr;
	while(**saveptr != 0 && strchr(delim, **saveptr)==NULL) {
		(*saveptr)++;
	}
	if(**saveptr == 0) {
		return start==*saveptr ? NULL : start;
	} else {
		**saveptr = 0;
		(*saveptr)++;
		return start;
	}
}
