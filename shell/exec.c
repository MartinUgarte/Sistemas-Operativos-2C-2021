#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	int idx;
	char key[ARGSIZE];
	char val[ARGSIZE];
	for (int i = 0; i < eargc; i++) {
		idx = block_contains(eargv[i], '=');
		// No puede tener el "=" en la primera posicion (0) o si es negativo no lo tiene.
		if (idx < 1)
			continue;
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], val, idx);
		// 1 significa sobreescribir (overwrite)
		if (setenv(key, val, 1) < 0)
			printf_debug("Fallo set env con key:%s y val:%s\n",
			             key,
			             val);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd;
	if (flags == O_RDONLY) {
		fd = open(file, flags | O_CLOEXEC);
	} else {
		fd = open(file,
		          flags | O_CLOEXEC | O_CREAT | O_TRUNC,
		          S_IRUSR | S_IWUSR);
	}
	if (fd == -1) {
		printf_debug("Fallo open con file:\n", file);
	}
	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC:
		// spawns a command
		e = (struct execcmd *) cmd;
		set_environ_vars(e->eargv, e->eargc);
		execvp(e->argv[0], e->argv);
		printf_debug("Fallo execcv con comando: %s\n", e->argv[0]);
		break;

	case BACK: {
		// runs a command in background
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		r = (struct execcmd *) cmd;
		int fd_in = -1;
		int fd_out = -1;
		int fd_err = -1;
		if (strlen(r->in_file) > 0) {
			fd_in = open_redir_fd(r->in_file, O_RDONLY);
			if (fd_in == -1)
				break;
			dup2(fd_in, STDIN_FILENO);
		}

		if (strlen(r->out_file) > 0) {
			fd_out = open_redir_fd(r->out_file, O_RDWR);
			if (fd_out == -1) {
				if (fd_in != -1)
					close(fd_in);
				break;
			}
			dup2(fd_out, STDOUT_FILENO);
		}

		if (strlen(r->err_file) > 0) {
			int idx = block_contains(r->err_file, '&');
			// Solo funciona si tiene el '&' en la primer posicion y
			// lo siguiente es un numero valido asociado a un fd.
			if (idx == 0 && strlen(r->err_file) > 1) {
				char *endptr;
				fd_err = (int) strtol(&(r->err_file[1]),
				                      &endptr,
				                      10);

				if (endptr == &(r->err_file[1])) {
					printf_debug("Redireccionamiento "
					             "invalido %s\n",
					             &(r->err_file[1]));
					fd_err = -1;
				}
				if (fd_err == -1)
					fd_err = open_redir_fd(&(r->err_file[1]),
					                       O_RDWR);
			}
			if (fd_err == -1)
				fd_err = open_redir_fd(r->err_file, O_RDWR);
			if (fd_err == -1) {
				if (fd_in != -1) {
					close(fd_in);
				}
				if (fd_out != -1) {
					close(fd_out);
				}
				break;
			}
			dup2(fd_err, STDERR_FILENO);
		}

		execvp(r->argv[0], r->argv);
		printf_debug("Fallo execcv con comando: %s\n", r->argv[0]);
		if (fd_in != -1)
			close(fd_in);
		if (fd_out != -1)
			close(fd_out);
		if (fd_err != -1)
			close(fd_err);
		break;
	}

	case PIPE: {
		// pipes two commands
		int pipe_fds[2];
		p = (struct pipecmd *) cmd;
		if (pipe(pipe_fds) == -1) {
			printf_debug("Fallo pipe\n");
			break;
		}

		int pid_lf = fork();
		if (pid_lf < 0) {
			printf_debug("Fallo el fork para el hijo izquierdo\n");
			break;
		}
		if (pid_lf == 0) {
			close(pipe_fds[READ]);
			dup2(pipe_fds[WRITE], STDOUT_FILENO);
			close(pipe_fds[WRITE]);
			exec_cmd(p->leftcmd);
			break;
		}

		int pid_rg = fork();
		if (pid_rg < 0) {
			printf_debug("Fallo el fork para el hijo derecho\n");
			break;
		}
		if (pid_rg == 0) {
			close(pipe_fds[WRITE]);
			dup2(pipe_fds[READ], STDIN_FILENO);
			close(pipe_fds[READ]);
			exec_cmd(p->rightcmd);
			break;
		}

		close(pipe_fds[WRITE]);
		close(pipe_fds[READ]);
		wait(NULL);
		wait(NULL);

		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);
		break;
	}
	}
}
