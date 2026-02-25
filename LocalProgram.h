#ifndef LOCALPROGRAM_H
#define LOCALPROGRAM_H

#include <stdbool.h>

bool command_exist(const char *cmd);
bool command_run(const char *cmd, const char *args, bool run_as_admin);

#endif
