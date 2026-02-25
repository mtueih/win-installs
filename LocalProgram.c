#include "localprogram.h"
#include "dynamic-string.h"
#include <stdlib.h>
#include <windows.h>
#include <stdio.h>

bool command_exist(const char *cmd) {
    const char *verify_cmd_suffix = ">nul 2>&1";
    const char *verify_cmd_args[] = {
        "--version",
        "-v",
        "-V",
        "--help",
        "-h",
        "-?"
    };
    const size_t verify_cmd_args_num = sizeof(verify_cmd_args) / sizeof(verify_cmd_args[0]);

    DString verify_cmd;
    size_t i;
    int result;
    
    verify_cmd = dstr_create();
    if (verify_cmd == NULL) {
        if (fprintf(
            stderr, 
            "[Failure] In function %s: Failed to create DString.\n", 
            __func__) < 0
        ) {
            perror("fprintf");
        }
        return false;
    }
    
    for (i = 0; i < verify_cmd_args_num; ++i) {
        result = dstr_assign_format(
            verify_cmd, "%s %s %s", 
            cmd, verify_cmd_args[i], verify_cmd_suffix
        );
        if (result != DSTRING_SUCCESS) {
            if (fprintf(
                stderr, 
                "\n[Failure (%d)] In function %s: Failed to assign format to DString.", 
                 result, __func__) < 0
            ) {
                perror("fprintf");
            }
            dstr_destroy(verify_cmd);
            return false;
        }

        if (system(dstr_cstr(verify_cmd)) == 0) {
            dstr_destroy(verify_cmd);
            return true;
        }
    }

    dstr_destroy(verify_cmd);
    return false;
}

bool command_run(const char *cmd, const char *args, bool run_as_admin) {
    SHELLEXECUTEINFO run_info;
    BOOL result;

    if (cmd == NULL) return false;

    run_info = (SHELLEXECUTEINFO){ 0 };

    run_info.cbSize = sizeof(SHELLEXECUTEINFO);
    run_info.fMask = SEE_MASK_NOCLOSEPROCESS;
    run_info.nShow = SW_SHOWNORMAL;

    run_info.lpFile = cmd;
    run_info.lpParameters = args;
    if (run_as_admin) {
        run_info.lpVerb = "runas";
    }
    
    result = ShellExecuteEx(&run_info);
    if (result) {
        WaitForSingleObject(run_info.hProcess, INFINITE);
        CloseHandle(run_info.hProcess);
    }

    return result;
}
