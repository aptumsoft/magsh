#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

#define MAX_LINE 1024
#define MAX_PATH 1024
#define MAX_ARGS 64

#define COLOR_GREEN "\033[1;32m"
#define COLOR_CYAN  "\033[1;36m"
#define COLOR_RESET "\033[0m"

void print_banner(){
    printf(COLOR_GREEN);
    //printf("HELLO and Welcome to Transformer Magnet Shell. "  
    //"If you have `magnet` installed, you can learn more "
    //"by typing `magnet tmsh-info`.\n");
    printf("This program is licensed under the GNU General Public License v3 (GPLv3).\n");
    printf("See <https://www.gnu.org/licenses/gpl-3.0.html> for details.\n");
    printf(COLOR_RESET);
}

int execute_builtin(char **args){
    if(args[0] == NULL){
        return 1;
    }

    if(strcmp(args[0], "exit") == 0){
        printf("Goodbye from TMSH! We hope to see you. :) \n");
        exit(0);
    }

    if(strcmp(args[0], "cd") == 0){
        if(args[1] == NULL){
            printf("TMSH: cd: missing args\n");
        }else{
            if(chdir(args[1]) != 0){
                perror("TMSH: cd");
            }
        }
        return 1;
    }

    return 0;
}

int main() {
    print_banner();
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    #ifdef SIGINFO
    signal(SIGINFO, SIG_IGN);
    #endif

    while (true) {
        char path[MAX_PATH];
        getcwd(path, sizeof(path));
        printf("\n");
        printf(COLOR_CYAN "TMSH");
        printf("[" COLOR_GREEN "%s" COLOR_CYAN "] >> " COLOR_RESET, path);

        char input[MAX_LINE];
        if(!fgets(input, MAX_LINE, stdin)) break;
        input[strcspn(input, "\n")] = 0;
        input[strcspn(input, "\r")] = 0;
        if (strlen(input) == 0) continue;

        char *args[MAX_ARGS];
        int i = 0;
        args[i] = strtok(input, " ");
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            args[i] = strtok(NULL, " ");
        }

        if (execute_builtin(args)) {
            continue;
        }

        int redirect_idx = -1;
        char *out_file = NULL;
        for (int j = 0; args[j] != NULL; j++) {
            if (strcmp(args[j], ">") == 0) {
                redirect_idx = j;
                if (args[j + 1] != NULL) {
                    out_file = args[j + 1];
                }
                args[j] = NULL;
                break;
            }
        }

        int pipe_index = -1;
        for (int j = 0; args[j] != NULL; j++) {
            if (strcmp(args[j], "|") == 0) {
                pipe_index = j;
                args[j] = NULL;
                break;
            }
        }

        if (pipe_index != -1) {
            int pipefd[2];
            if (pipe(pipefd) == -1) {
                perror("TMSH: pipe failed");
                continue;
            }

            char **cmd1 = args;
            char **cmd2 = &args[pipe_index + 1];

            pid_t pid1 = fork();
            if (pid1 == 0) {
                signal(SIGINT, SIG_DFL);
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
                execvp(cmd1[0], cmd1);
                fprintf(stderr, "TMSH: command not found: %s\n", cmd1[0]);
                exit(1);
            }

            pid_t pid2 = fork();
            if (pid2 == 0) {
                signal(SIGINT, SIG_DFL);
                if (redirect_idx != -1 && out_file != NULL && cmd2[0] != NULL) {
                    int fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) {
                        perror("TMSH: open failed");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                dup2(pipefd[0], STDIN_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
                execvp(cmd2[0], cmd2);
                fprintf(stderr, "TMSH: command not found: %s\n", cmd2[0]);
                exit(1);
            }

            close(pipefd[0]);
            close(pipefd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        } else {
            pid_t pid = fork();
            if (pid == 0) {
                signal(SIGINT, SIG_DFL);
                signal(SIGTSTP, SIG_DFL);
                signal(SIGQUIT, SIG_DFL);
                #ifdef SIGINFO
                signal(SIGINFO, SIG_DFL);
                #endif
                if (redirect_idx != -1 && out_file != NULL) {
                    int fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) {
                        perror("TMSH: open failed");
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                execvp(args[0], args);
                fprintf(stderr, "TMSH: command not found: %s\n", args[0]);
                exit(1);
            } else if (pid > 0) {
                int status;
                waitpid(pid, &status, 0);
            } else {
                perror("TMSH: fork failed");
            }
        }
    }

    return 0;
}
