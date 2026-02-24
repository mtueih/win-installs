#include "LocalProgram.h"
#include <stdio.h>

int main(void) {

    if (isCommandExist("powershell")) {
        printf("test\n");
        runProgram("powershell", "-Command \"winget list \"Firefox\";Read-Host\"", false);
    } else {
        printf("Powershell is not found.\n");
    }

    return 0;
}