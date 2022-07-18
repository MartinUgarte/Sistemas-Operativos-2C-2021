#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <sys/wait.h>

#define READ 0
#define WRITE 1

static int
get_number(int argc, char *argv[])
{
	char *endptr;

	if (argc != 2) {
		fprintf(stderr, "Error, se necesita un único parámetro que sea un entero natural mayor a 1\n");
		return -1;
	}

	long int n = strtol(argv[1], &endptr, 10);
	if ((errno == ERANGE && (n == LONG_MAX || n == LONG_MIN)) ||
	    (errno != 0 && n == 0)) {
		perror("strtol error");
		return -1;
	}

	if (endptr == argv[1] || *endptr != '\0' || n < 2) {
		fprintf(stderr, "\"Error, se necesita un único parámetro que sea un entero natural mayor a 1\n");
		return -1;
	}

	return n;
}

static void
filter(int read_fd)
{
	int prime;
	if (read(read_fd, &prime, sizeof(int)) <= 0) {
		return;
	}
	printf("primo: %d\n", prime);

	int right_pipe[2];

	if (pipe(right_pipe) == -1) {
		perror("Error creando pipes");
		exit(-1);
	}

	int pid = fork();

	if (pid < 0) {
		perror("Fork error");
		exit(-1);
	}

	if (pid == 0) {
		close(read_fd);
		close(right_pipe[WRITE]);
		filter(right_pipe[READ]);
		exit(0);
	} else {
		close(right_pipe[READ]);
		int n;
		while (read(read_fd, &n, sizeof(int)) > 0) {
			if ((n % prime) != 0) {
				if (write(right_pipe[WRITE], &n, sizeof(int)) < 0) {
					perror("Write error parent->child");
					exit(-1);
				}
			}
		}
		close(read_fd);
		close(right_pipe[WRITE]);
		wait(NULL);
	}
}

int
main(int argc, char *argv[])
{
	long int n = get_number(argc, argv);
	if (n < 0)
		return -1;

	int pc_fds[2];
	if (pipe(pc_fds) == -1) {
		perror("Error creando pipes");
		return -1;
	}

	int pid = fork();

	if (pid < 0) {
		perror("Fork error");
		return -1;
	}

	if (pid == 0) {
		close(pc_fds[WRITE]);
		filter(pc_fds[READ]);
	} else {
		close(pc_fds[READ]);
		for (int i = 2; i <= n; i++) {
			if (write(pc_fds[WRITE], &i, sizeof(int)) < 0) {
				perror("Write error parent->child");
				return -1;
			}
		}
		close(pc_fds[WRITE]);
		wait(NULL);
	}
	return 0;
}
