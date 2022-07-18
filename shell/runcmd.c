#include "runcmd.h"

int status = 0;
struct cmd *parsed_pipe;

void
check_zombies()
{
	int pid;
	do {
		pid = waitpid(-1, &status, WNOHANG);
		if (pid != 0 && pid != -1)
			printf_debug("Termino proceso hijo de segundo plano "
			             "con pid: %d\n",
			             pid);
	} while (pid != 0 && pid != -1);
}

// runs the command in 'cmd'
int
run_cmd(char *cmd)
{
	pid_t p;
	struct cmd *parsed;

	// Puedo tener hijos zombies por haberlos corrido
	// en segundo plano y no esperar que terminen.
	check_zombies();

	// if the "enter" key is pressed
	// just print the promt again
	if (cmd[0] == END_STRING)
		return 0;

	// "cd" built-in call
	if (cd(cmd))
		return 0;

	// "exit" built-in call
	if (exit_shell(cmd))
		return EXIT_SHELL;

	// "pwd" buil-in call
	if (pwd(cmd))
		return 0;

	// parses the command line
	parsed = parse_line(cmd);

	// forks and run the command
	if ((p = fork()) == 0) {
		// keep a reference
		// to the parsed pipe cmd
		// so it can be freed later
		if (parsed->type == PIPE)
			parsed_pipe = parsed;

		exec_cmd(parsed);
		exit(-1);
	}

	// store the pid of the process
	parsed->pid = p;

	// background process special treatment
	// Hint:
	// - check if the process is
	//		going to be run in the 'back'
	// - print info about it with
	// 	'print_back_info()'

	if (parsed->type == BACK) {
		print_back_info(parsed);
	} else {
		// waits for the process to finish
		waitpid(p, &status, 0);
		print_status_info(parsed);
	}
	free_command(parsed);
	return 0;
}
