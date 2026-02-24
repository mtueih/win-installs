#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "LocalProgram.h"
#include "dynamic-string.h"

#define MAX_TASKS 9
#define CHAR_BUFFER_SIZE 256

char g_inputBuffer[CHAR_BUFFER_SIZE];

typedef enum {
    ONE_COMMAND,
    BATCH_COMMAND
} TaskType;

typedef union {
    const char *commands;
    void (*taskfunc)(void);
} TaskAction;

typedef struct {
    TaskType type;
    const char *description;
    TaskAction action;
    bool run_as_admin;
} Task;

void task8(void);
void task9(void);

const Task g_tasks[MAX_TASKS] = {
    { 
        ONE_COMMAND, 
        "开启 UTF-8", 
        { .commands = "Set-ItemProperty -Path \"HKLM:\\SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage\" -Name \"ACP\" -Value \"65001\"; Set-ItemProperty -Path \"HKLM:\\SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage\" -Name \"MACCP\" -Value \"65001\"; Set-ItemProperty -Path \"HKLM:\\SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage\" -Name \"OEMCP\" -Value \"65001\"" 
        }, 
        true 
    },
    { 
        ONE_COMMAND, 
        "修改脚本策略为「RemoteSigned」（当前用户）", 
        { .commands = "Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser" }, 
        false 
    },
    { 
        ONE_COMMAND, 
        "安装「Chocolatey」", 
        { .commands = "Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString(\'https://community.chocolatey.org/install.ps1\'))" }, 
        true 
    },
    { 
        ONE_COMMAND, 
        "安装「Scoop」", 
        { .commands = "Set-ExecutionPolicy Bypass -Scope Process -Force; Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression" }, 
        false 
    },
    { 
        ONE_COMMAND, 
        "Chocolatey 安装「Cloudfalre Warp」", 
        { .commands = "choco install warp  -y --ia 'ORGANIZATION=saibohuofoyyds'" }, 
        true 
    },
    { 
        ONE_COMMAND, 
        "Scoop 安装基础工具（Git、7Zip、dark 和 innounp 等）", 
        { .commands = "scoop install git 7zip dark innounp" }, 
        false 
    },
    { 
        ONE_COMMAND, 
        "Scoop 添加常用「Bucket」", 
        { .commands = "scoop bucket add extras" }, 
        false 
    },
    { 
        BATCH_COMMAND, 
        "安装必备软件（Windows 必装软件.md）", 
        { .taskfunc = task8 }, 
        false 
    },
    { 
        BATCH_COMMAND, 
        "依次执行上述 1-8", 
        { .taskfunc = task9 }, 
        false 
    }
};


void printMenu(void);
int getChoice(void);
void run_task(const Task *task);
void task_one_command(const Task *task);
void runByPowershell(const char *command, bool admin, bool wait);

int main(void) {
    system("chcp 65001");

    int choice = 1;

    do {
        choice = getChoice();
        if (!choice) break;

        run_task(&g_tasks[choice - 1]);

    } while (choice);
    
    printf("感谢使用！");

    return 0;
}

void printMenu(void) {

    int i;

    for (i = 0; i < MAX_TASKS; ++i) {
        printf("\n【%d】%s", i + 1, g_tasks[i].description);
    }

    printf("\n【0】退出\n");
}

int getChoice(void) {
    int choice = 0;
    int ch;

    system("cls");
    printMenu();
    printf("\n请输入你的选择：");

    while (scanf("%d", &choice) != 1 || choice < 0 || choice > MAX_TASKS) {
        system("cls");
        printMenu();
        printf("\n输入有误！请重新输入你的选择：");
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    }

    return choice;
}

void runByPowershell(const char *command, bool admin, bool wait) {
    if (!command) return;
    
    DString cmd_args_str = dstr_create();

    if (!cmd_args_str) {
        fprintf(stderr, "Failed to create dynamic string for command arguments.\n");
        return;
    }

    dstr_assign_format(cmd_args_str, "-Command \"%s%s\"", command, 
        wait ? " ; Read-Host \"按 Enter 键退出...\"" : ""
    );

    runProgram("powershell", dstr_cstr(cmd_args_str), true);

    dstr_destroy(cmd_args_str);
}

void task_one_command(const Task *task) {
    printf("\n命令「%s」\n将使用 Windows Powershell 以%s身份执行。 \n", 
        task->action.commands,
        task->run_as_admin ? "管理员" : "当前用户"
    );
    
    runByPowershell(task->action.commands, task->run_as_admin, true);
}

void run_task(const Task *task) {
    bool retry = false;
    int ch;
    char input[2] = "N";

    do {
        system("cls");
        printf("\n%s", task->description);

        if (task->type == ONE_COMMAND) {
             task_one_command(task);
        } else if (task->type == BATCH_COMMAND) {
             task->action.taskfunc();
        } else {
            fprintf(stderr, "未知的任务类型！\n");
        }

        printf("\n任务【%s】\n执行完毕。是否重试？（y/N）", task->description);
        scanf("%1s", input);
        retry = (input[0] == 'y' || input[0] == 'Y');
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    } while (retry);
}

void task8(void) {
    const char * const list_file_path = "Windows 必装软件.md";
    FILE *list_file;
    DString line;
    size_t len;
    bool in_softare_section;
    bool in_install_command_section;
    bool retry;
    char input[2] = "N";
    int ch;

    printf("\n读取文件「%s」", list_file_path);
    list_file = fopen(list_file_path, "r");
    if (!list_file) {
        fprintf(stderr, "\n无法打开文件：「%s」", list_file_path);
        return;
    }

    line = dstr_create();
    if (!line) {
        fprintf(stderr, "\n无法创建动态字符串来读取文件内容。");
        fclose(list_file);
        return;
    }
    in_softare_section = false;
    in_install_command_section = false;
    retry = false;

    while (fgets(g_inputBuffer, CHAR_BUFFER_SIZE, list_file) != NULL) {
        // 如果没有完整读取一行，则将当前内容追加到 line 中，并继续读取
        len = strlen(g_inputBuffer);
        if (g_inputBuffer[len - 1] != '\n') {
            if (!dstr_append_cstr(line, g_inputBuffer)) {
                fprintf(stderr, "\n无法追加读取的内容到动态字符串。");
                dstr_destroy(line);
                fclose(list_file);
                return;
            }
            continue;
        }

        // 读取到完整的一行，此时去除默认 '\n'
        g_inputBuffer[len - 1] = '\0';
        if (!dstr_append_cstr(line, g_inputBuffer)) {
            fprintf(stderr, "\n无法追加读取的内容到动态字符串。");
            dstr_destroy(line);
            fclose(list_file);
            return;
        }
        
        // 处理 line
        // 一级标题，视为任务流名称
        if (dstr_start_with_cstr(line, "# ")) {
            printf("\n安装流【%s】", dstr_cstr(line) + 2);
        } else if (dstr_start_with_cstr(line, "## ")) {
            // 二级标题，视为软件分类
            printf("\n分类「%s」", dstr_cstr(line) + 3);
        } else if (dstr_start_with_cstr(line, "### ")) {
            // 三级标题，视为软件名称
            printf("\n软件「%s」", dstr_cstr(line) + 4);
            in_softare_section = true;
        } else if (in_softare_section && !dstr_start_with_cstr(line, "- ")) {
            // 在软件名称下的非列表项，视为软件描述
            printf("\n描述：%s", dstr_cstr(line));
        } else if (dstr_equal_cstr(line, "```powershell") && in_softare_section) {
            // 在软件名称下遇到 PowerShell 代码块的开始，视为安装命令的开始
            in_install_command_section = true;
        } else if (dstr_equal_cstr(line, "```") 
            && in_install_command_section 
            && in_softare_section
        ) {
            // 在软件名称下遇到 PowerShell 代码块的结束
            in_install_command_section = false;
            in_softare_section = false;
        } else if (in_install_command_section) {
            // 在安装命令部分，视为安装命令的一行
            printf("\n安装命令：「%s」", dstr_cstr(line));
            do {
                runByPowershell(dstr_cstr(line), 
                    dstr_start_with_cstr(line, "scoop") ? false : true,
                    false
                );
                printf("\n安装命令「%s」\n执行完毕。是否重试？（y/N）", dstr_cstr(line));
                scanf("%1s", input);
                retry = (input[0] == 'y' || input[0] == 'Y');
                while ((ch = getchar()) != '\n' && ch != EOF)
                    ;
            } while (retry);
            
        }
        
        dstr_clear(line);
    }

}

void task9(void) {
    size_t i;

    for (i = 0; i < MAX_TASKS - 1; ++i) {
        run_task(&g_tasks[i]);
    }
}