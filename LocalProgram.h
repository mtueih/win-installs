#ifndef LOCALPROGRAM_H
#define LOCALPROGRAM_H

#include <stdbool.h>

bool isCommandExist(const char *command);
bool runProgram(const char *program, const char *args, bool runAsAdmin);

#endif
