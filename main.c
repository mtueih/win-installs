#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "LocalProgram.h"
#include "dynamic-string.h"

#define MAX_TASKS 9
#define CHAR_BUFFER_SIZE 256

typedef enum {
    ONE_COMMAND,
    MULTI_COMMAND,
    FUNCTION
} TaskType;

typedef union {
    const char *command;
    void (*taskfunc)(void);
} TaskAction;

typedef struct {
    TaskType type;
    const char *description;
    TaskAction action;
    bool run_as_admin;
} Task;

void task_8(void);
void task_9(void);

const Task g_tasks[MAX_TASKS] = {
    { 
        ONE_COMMAND, 
        "开启 UTF-8", 
        { .command = "Set-ItemProperty -Path \"HKLM:\\SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage\" -Name \"ACP\" -Value \"65001\"; Set-ItemProperty -Path \"HKLM:\\SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage\" -Name \"MACCP\" -Value \"65001\"; Set-ItemProperty -Path \"HKLM:\\SYSTEM\\CurrentControlSet\\Control\\Nls\\CodePage\" -Name \"OEMCP\" -Value \"65001\"" }, 
        true
    },
    { 
        ONE_COMMAND, 
        "修改脚本策略为「RemoteSigned」（当前用户）", 
        { .command = "Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser" }, 
        false 
    },
    { 
        ONE_COMMAND,
        "安装「Chocolatey」", 
        { .command = "Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString(\'https://community.chocolatey.org/install.ps1\'))" }, 
        true
    },
    { 
        ONE_COMMAND, 
        "安装「Scoop」", 
        { .command = "Set-ExecutionPolicy Bypass -Scope Process -Force; Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression" }, 
        false
    },
    {
        ONE_COMMAND, 
        "Chocolatey 安装「Cloudfalre Warp」", 
        { .command = "choco install warp  -y --ia 'ORGANIZATION=saibohuofoyyds'" }, 
        true
    },
    { 
        ONE_COMMAND,
        "Scoop 安装基础工具（Git、7Zip、dark 和 innounp 等）",
        { .command = "scoop install git 7zip dark innounp" },
        false
    },
    { 
        ONE_COMMAND,
        "Scoop 添加常用「Bucket」",
        { .command = "scoop bucket add extras nonportable nerd-fonts" },
        false
    },
    { 
        FUNCTION,
        "安装必备软件（Windows 必装软件.md）",
        { .taskfunc = task_8 },
        false
    },
    { 
        FUNCTION, 
        "依次执行上述 1-8", 
        { .taskfunc = task_9 }, 
        false
    }
};


inline void printMenu(void);
int getChoice(void);
void task_run(const Task *task, int task_num);
void task_one_cmd_run(const Task *task);
void powershell_run(const char *cmd, bool admin, bool wait);

int main(void) {
    system("chcp 65001");

    int choice = 1;

    do {
        choice = getChoice();
        if (choice == 0) break;
        task_run(&g_tasks[choice - 1], choice);
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

void powershell_run(const char *cmd, bool admin, bool wait) {
    DString cmd_args;
    int result;

    if (cmd == NULL) return;

    cmd_args = dstr_create();
    if (cmd_args != DSTRING_SUCCESS) {
        if (fprintf(
            stderr, 
            "[Failure] In function %s: Failed to create DString.\n", 
            __func__) < 0
        ) {
            perror("fprintf");
        }
        return;
    }

    result = dstr_assign_format(
        cmd_args, "-Command \"%s%s\"", 
        cmd, wait ? " ; Read-Host \"Press Enter to exit...\"" : ""
    );

    if (result != DSTRING_SUCCESS) {
        if (fprintf(
            stderr, 
            "\n[Failure (%d)] In function %s: Failed to assign format to DString.", 
             result, __func__) < 0
        ) {
            perror("fprintf");
        }
        dstr_destroy(cmd_args);
        return;
    }

    command_run("powershell", dstr_cstr(cmd_args), admin);

    dstr_destroy(cmd_args);
}

void task_one_cmd_run(const Task *task) {
    printf("\n命令「%s」\n将使用 Windows Powershell 以%s身份执行。 \n", 
        task->action.command,
        task->run_as_admin ? "管理员" : "当前用户"
    );
    
    powershell_run(task->action.command, task->run_as_admin, true);
}

void task_run(const Task *task, int task_num) {
    bool retry = false;
    int ch;

    do {
        system("cls");
        printf("\n【%d】%s", task_num, task->description);

        if (task->type == ONE_COMMAND) {
             task_one_cmd_run(task);
        } else if (task->type == FUNCTION) {
             task->action.taskfunc();
        } else {
            fprintf(stderr, "未知的任务类型！\n");
        }

        printf("\n任务【%s】\n执行完毕。是否重试？（y/N）", task->description);

        retry = ((ch = getchar()) == 'y' || ch == 'Y');
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
    } while (retry);
}

void task_8(void) {
    const char *list_file_path = "Windows 必装软件.md";
    FILE *list_file;
    char buffer[CHAR_BUFFER_SIZE];
    DString line;
    bool in_softare_section;
    bool in_install_command_section;
    bool retry;
    int ch;
    int result;

    printf("\n读取文件「%s」", list_file_path);
    list_file = fopen(list_file_path, "r");
    if (list_file == NULL) {
        perror("fopen");
        return;
    }

    line = dstr_create();
    if (line == NULL) {
        if (fprintf(
            stderr, 
            "[Failure] In function %s: Failed to create DString.\n", 
            __func__) < 0
        ) {
            perror("fprintf");
        }
        fclose(list_file);
        return;
    }

    in_softare_section = false;
    in_install_command_section = false;
    retry = false;

    while (fgets(buffer, CHAR_BUFFER_SIZE, list_file) != NULL) {
        // 如果没有完整读取一行，则将当前内容追加到 line 中，并继续读取
        result = dstr_append_cstr(line, buffer);
        if (result != DSTRING_SUCCESS) {
            if (fprintf(
                stderr, 
                "\n[Failure (%d)] In function %s: Failed to append cstr to DString.", 
                 result, __func__) < 0
            ) {
                perror("fprintf");
            }
            dstr_destroy(line);
            fclose(list_file);
            return;
        }
        // 判断有没有读完一行
        if (!dstr_end_with_cstr(line, "\n")) {
            continue;
        }
        // 读取到完整的一行，此时去除默认 '\n'
        result = dstr_erase(line, dstr_length(line) - 1, 1);
        if (result != DSTRING_SUCCESS) {
            if (fprintf(
                stderr, 
                "\n[Failure (%d)] In function %s: Failed to erase from DString.", 
                 result, __func__) < 0
            ) {
                perror("fprintf");
            }
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
                powershell_run(dstr_cstr(line), 
                    dstr_start_with_cstr(line, "scoop") ? false : true,
                    false
                );
                printf("\n安装命令「%s」\n执行完毕。是否重试？（y/N）", dstr_cstr(line));

                retry = ((ch = getchar()) == 'y' || ch == 'Y');
                while ((ch = getchar()) != '\n' && ch != EOF)
                    ;
            } while (retry);
            
        }
        
        dstr_clear(line);
    }

    dstr_destroy(line);
    fclose(list_file);
}

void task_9(void) {
    int i;

    for (i = 0; i < MAX_TASKS - 1; ++i) {
        task_run(&g_tasks[i], i + 1);
    }
}