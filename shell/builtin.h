#ifndef BUILTIN_H
#define BUILTIN_H

#include "defs.h"
#include "utils.h"

extern int status;
extern char promt[PRMTLEN];

int cd(char *cmd);

int exit_shell(char *cmd);

int pwd(char *cmd);

#endif  // BUILTIN_H
