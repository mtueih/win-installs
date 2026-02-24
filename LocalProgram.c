#include "localprogram.h"
#include "dynamic-string.h"
#include <stdlib.h>
#include <windows.h>
#include <shellapi.h>
#include <stdio.h>

bool isCommandExist(const char *command) {
    static const char * const verificationCommandSuffix = ">nul 2>&1";
    static const char * const verificationCommandArguments[] = {
        "--version",
        "-v",
        "-V",
        "--help",
        "-h",
        "-?"
    };

    DString verificationCommand;
    size_t i;
    int result;
    
    verificationCommand = dstr_create();

    if (!verificationCommand) { 
        fprintf(stderr, "Failed to create dynamic string for command verification.\n");
        return false;
    }
    
    for (i = 0; i < sizeof(verificationCommandArguments) 
            / sizeof(verificationCommandArguments[0]); ++i
    ) {
        result = dstr_assign_format(verificationCommand, 
            "%s %s %s", command, verificationCommandArguments[i], verificationCommandSuffix
        );

        if (result != DSTRING_SUCCESS) {
            fprintf(stderr, "Error code %d\n", result);
            dstr_destroy(verificationCommand);
            return false;
        }

        if ((system(dstr_cstr(verificationCommand)) == 0)) {
            puts(dstr_cstr(verificationCommand));
            dstr_destroy(verificationCommand);
            return true;
        }
    }

    dstr_destroy(verificationCommand);
    return false;
}

bool runProgram(const char *program, const char *args, bool runAsAdmin) {
    SHELLEXECUTEINFO runInfo = { 0 };
    BOOL result;

    if (!program) return false;

    runInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    runInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    runInfo.nShow = SW_SHOWNORMAL;

    runInfo.lpFile = program;
    runInfo.lpParameters = args;
    
    if (runAsAdmin) {
        runInfo.lpVerb = "runas";
    }
    
    result = ShellExecuteEx(&runInfo);

    if (result) {
        WaitForSingleObject(runInfo.hProcess, INFINITE);
        CloseHandle(runInfo.hProcess);
    }

    return result;
}
