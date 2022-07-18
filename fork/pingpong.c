#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define READ 0
#define WRITE 1

int
main(void)
{
	printf("Hola, soy PID %d:\n", getpid());

	int pc_fds[2], cp_fds[2];
	srand(time(NULL));

	if (pipe(pc_fds) == -1 || pipe(cp_fds) == -1) {
		perror("Error creando pipes");
		return -1;
	}
	printf("  - primer pipe me devuelve: [%d, %d]\n",
	       pc_fds[READ],
	       pc_fds[WRITE]);
	printf("  - segundo pipe me devuelve: [%d, %d]\n",
	       cp_fds[READ],
	       cp_fds[WRITE]);

	int pid = fork();

	if (pid < 0) {
		perror("Fork error");
		return -1;
	}

	if (pid == 0) {
		close(cp_fds[READ]);
		close(pc_fds[WRITE]);
		long int recv;
		if (read(pc_fds[READ], &recv, sizeof(long int)) < 0) {
			perror("Read error child<-parent");
			return -1;
		}

		printf("\nDonde fork me devuelve %d:\n", pid);
		printf("  - getpid me devuelve: %d\n", getpid());
		printf("  - getppid me devuelve: %d\n", getppid());
		printf("  - recibo valor %ld vía fd=%d\n", recv, pc_fds[READ]);
		printf("  - reenvío valor en fd=%d y termino\n", cp_fds[WRITE]);

		if (write(cp_fds[WRITE], &recv, sizeof(long int)) < 0) {
			perror("Write error child->parent");
			return -1;
		}
		close(pc_fds[READ]);
		close(cp_fds[WRITE]);
	} else {
		close(pc_fds[READ]);
		close(cp_fds[WRITE]);
		long int val = random();

		printf("\nDonde fork me devuelve %d:\n", pid);
		printf("  - getpid me devuelve: %d\n", getpid());
		printf("  - getppid me devuelve: %d\n", getppid());
		printf("  - random me devuelve: %ld\n", val);
		printf("  - envío valor %ld a través de fd=%d\n",
		       val,
		       pc_fds[WRITE]);

		if (write(pc_fds[WRITE], &val, sizeof(long int)) < 0) {
			perror("Write error parent->child");
			return -1;
		}

		long int recv;
		if (read(cp_fds[READ], &recv, sizeof(long int)) < 0) {
			perror("Read error parent<-child");
			return -1;
		}

		printf("\nHola, de nuevo PID %d:\n", getpid());
		printf("  - recibí valor %ld vía fd=%d\n", recv, cp_fds[READ]);

		close(cp_fds[READ]);
		close(pc_fds[WRITE]);
	}

	return 0;
}