#include <kernel/panic.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

//128 chars ought to be enough for anyone
#define BUFFER_SIZE 32

static char* readline();
static void execute(char* line );

// Builtin commands
static void builtin_help();

char* builtin_cmd[] = {
	"HELP"
};

void (*builtin_func[])() = {
	&builtin_help
};

#define NB_CMDS (sizeof(builtin_cmd) / sizeof(builtin_cmd[0]))

void test_shell() {
	while(true) {
		printf("\n> ");
		char* line = readline();
		printf("\n");
		execute(line);
		free(line);
	}
}

static char* readline() {
	size_t buffer_size = BUFFER_SIZE;
	char* buffer = malloc(buffer_size);
	size_t position = 0;
	int c;

	if(buffer == NULL) {
		panic("out of memory");
	}

	while(true) {
		c = getchar();
		if(c == EOF || c == '\n') {
			buffer[position] = 0;
			return buffer;
		} else {
			buffer[position++] = c;
			printf("%c", c);
		}
		if(position == buffer_size) {
			buffer_size += BUFFER_SIZE;
			buffer = realloc(buffer, buffer_size);
			if(buffer == NULL) {
				panic("out of memory in realloc");
			}
		}
	}
}

static void execute(char* line ) {
	if(line == NULL || strcmp(line, "")==0) {
		return;
	}
	for(size_t i=0; i<NB_CMDS; i++) {
		if(strcmp(line, builtin_cmd[i]) == 0) {
			(*builtin_func[i])();
			return;
		}
	}
	printf("Invalid command");
	return;
}

static void builtin_help() {
	printf("Valid commands:\n");
	for(size_t i=0; i<NB_CMDS; i++) {
		printf(" - %s\n", builtin_cmd[i]);
	}
}