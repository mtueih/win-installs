#include "LocalProgram.h"
#include <stdio.h>

int main(void) {

    if (command_exist("powershell")) {
        printf("test\n");
        command_run("powershell", "-Command \"winget list \"Firefox\";Read-Host\"", false);
    } else {
        printf("Powershell is not found.\n");
    }

    return 0;
}