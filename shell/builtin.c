#include "builtin.h"

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit") == 0) {
		status = 0;
		return EXIT_SHELL;
	}
	return 0;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (strncmp(cmd, "cd", 2) != 0)
		return 0;
	char *new_dir;
	if (strlen(cmd) == 2) {
		new_dir = getenv("HOME");
		if (new_dir == NULL) {
			status = 1;
			return 1;
		}
	} else {
		new_dir = split_line(cmd, SPACE);
	}

	if (chdir(new_dir) < 0) {
		printf_debug("error en chdir, new dir: %s\n", new_dir);
		status = 1;
	} else {
		char prm[PRMTLEN];
		if (getcwd(prm, sizeof(prm)) == NULL) {
			status = 1;
			return 1;
		}
		snprintf(promt, sizeof promt, "(%s)", prm);
		status = 0;
	}
	return 1;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd") == 0) {
		char dir[PRMTLEN];
		if (getcwd(dir, sizeof(dir)) != NULL) {
			printf("%s\n", dir);
			status = 0;
			return 1;
		}
		printf_debug("Error en getcwd");
		status = 1;
	}
	return 0;
}
