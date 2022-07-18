#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#ifndef NARGS
#define NARGS 4
#endif

static void
process(char **args)
{
	int pid = fork();
	if (pid == 0) {
		execvp(args[0], args);
		perror("execvp error");
		exit(-1);
	} else {
		wait(NULL);
	}
}

static void
fill_args(char **args, char *str, int read)
{
	char *token = NULL;
	token = strtok(str, "\n");
	int i = 1;
	while (token != NULL) {
		args[i] = token;
		i++;
		token = strtok(NULL, "\n");
	}
	args[1 + read] = NULL;
}

int
main(int argc, char *argv[])
{
	if (argc <= 1) {
		fprintf(stderr, "Error, se necesita un argumento\n");
		return -1;
	}
	char *args[NARGS + 2];
	args[0] = argv[1];
	char str[1000];
	strncpy(str, "", sizeof(str));
	char *line = NULL;
	size_t len = 0;
	int read = 0;

	while (read < NARGS && getline(&line, &len, stdin) != -1) {
		strcat(str, line);
		read++;
		if (read == NARGS) {
			fill_args(args, str, read);
			process(args);
			read = 0;
			strncpy(str, "", sizeof(str));
		}
	}
	if (read > 0) {
		fill_args(args, str, read);
		process(args);
	}
	return 0;
}