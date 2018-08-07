#include <kernel/panic.h>
#include <kernel/mm.h>
#include <kernel/sched.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define LINE_BUFFER_SIZE 32
#define ARGS_BUFFER_SIZE 4
#define ARGS_SEPARATOR " "

static char* readline();
static char** parseline(char* line, int* argc);
static void execute(int argc, char** argv);

// Builtin commands
static void builtin_help(int argc, char** argv);
static void builtin_echo(int argc, char** argv);
static void builtin_pcb(int argc, char** argv);

char* builtin_cmd[] = {
	"help",
	"echo",
	"pcb"
};

void (*builtin_func[])(int, char**) = {
	&builtin_help,
	&builtin_echo,
	&builtin_pcb
};

#define NB_CMDS (sizeof(builtin_cmd) / sizeof(builtin_cmd[0]))

void test_shell() {
	while(true) {
		printf("\n> ");
		char* line = readline();
		int argc;
		char** argv = parseline(line, &argc);
		printf("\n");
		execute(argc, argv);
		free(argv);
		free(line);
	}
}

static char** parseline(char* line, int* argc) {
	size_t args_size = ARGS_BUFFER_SIZE;
	*argc = 0;
	char** argv = malloc(args_size * sizeof(char*));
	char* arg;
	char* saveptr;

	if(argv == NULL) {
		panic("out of memory");
	}
	arg = strtok_r(line, ARGS_SEPARATOR, &saveptr);
	while(arg != NULL) {
		argv[(*argc)++] = arg;
		if(*argc == args_size) {
			args_size += ARGS_BUFFER_SIZE;
			argv = realloc(argv, args_size * sizeof(char*));
			if(argv == NULL) {
				panic("out of memory in realloc");
			}
		}
		arg = strtok_r(NULL, ARGS_SEPARATOR, &saveptr);
	}
	return argv;
}

static char* readline() {
	size_t buffer_size = LINE_BUFFER_SIZE;
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
			buffer_size += LINE_BUFFER_SIZE;
			buffer = realloc(buffer, buffer_size);
			if(buffer == NULL) {
				panic("out of memory in realloc");
			}
		}
	}
}

static void execute(int argc, char** argv) {
	if(argc == 0 || strcmp(argv[0], "")==0) {
		return;
	}
	for(size_t i=0; i<NB_CMDS; i++) {
		if(strcmp(argv[0], builtin_cmd[i]) == 0) {
			(*builtin_func[i])(argc, argv);
			return;
		}
	}
	printf("Invalid command");
	return;
}

static void builtin_help(int argc, char** argv) {
	printf("Valid commands:\n");
	for(size_t i=0; i<NB_CMDS; i++) {
		printf(" - %s\n", builtin_cmd[i]);
	}
}

static void builtin_echo(int argc, char** argv) {
	for(int i=1; i<argc; i++) {
		printf("%s ", argv[i]);
	}
}

static void builtin_pcb(int argc, char** argv) {
	process_control_block_t *pcb = current_process_control_block();

	printf("PID: %u\nHeap from 0x%X to 0x%X (%u bytes)\nRoot dir: %X\nWorking dir: %X",
	pcb->tgid,
	pcb->mm_info->kernel_info->heap_start,
	pcb->mm_info->kernel_info->heap_end,
	(pcb->mm_info->kernel_info->heap_end - pcb->mm_info->kernel_info->heap_start),
	pcb->vfs_info->root,
	pcb->vfs_info->cwd
	);

}
