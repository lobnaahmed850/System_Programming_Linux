#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUF_SIZE 1024

int picoshell_main(int argc, char *argv[]) {
    int lastStatus = 0;
    char buf[BUF_SIZE];

    while (1) {
        printf("Pico shell prompt > \n");
        fflush(stdout); // control order between stdout, stdin, stderr

        if (fgets(buf, BUF_SIZE, stdin) == NULL) {
            printf("\n");
            break;
        }

        buf[strlen(buf) - 1] = 0; // terminate string
        if (strlen(buf) == 0) {   // enter key -> print prompt again
            printf("\n");
            continue;
        }

        printf("\n");

        int argCnt = 0;
        char **argvArr = NULL; // dynamic allocation: point to strings of different lengths

        char *token = strtok(buf, " "); // cuts string at spaces
        while (token != NULL) {
            argvArr = (char **)realloc(argvArr, (argCnt + 2) * sizeof(char *)); // changes size
            argvArr[argCnt] = strdup(token);
            argCnt++;
            token = strtok(NULL, " ");
        }
        argvArr[argCnt] = NULL; // argv ends with NULL

        // built-in commands

        // echo
        if (strcmp(argvArr[0], "echo") == 0) {
            for (int i = 1; argvArr[i] != NULL; i++) {
                printf("%s", argvArr[i]);
                if (argvArr[i + 1]) {
                    printf(" ");
                }
            }
            printf("\n");
        }

        // pwd
        else if (strcmp(argvArr[0], "pwd") == 0) {
            char cwdBuff[1024];
            if (getcwd(cwdBuff, sizeof(cwdBuff)) != NULL) {
                printf("%s\n", cwdBuff);
                lastStatus = 0; // success
            } else {
                perror("pwd");
                lastStatus = 1; // failure
            }
        }

        // cd
        else if (strcmp(argvArr[0], "cd") == 0) {
            if (argvArr[1] == NULL) {
                fprintf(stderr, "cd: missing argument\n");
                lastStatus = 1; // failure
            } else if (chdir(argvArr[1]) != 0) {
                fprintf(stderr, "\ncd: %s: No such file or directory", argvArr[1]);
                lastStatus = 1; // failure
            } else {
                lastStatus = 0; // success
            }
        }

        // exit
        else if (strcmp(argvArr[0], "exit") == 0) {
            printf("Good Bye\n");
            break; // shell terminates
        }

        // external commands
        else {
            fflush(stdout); // evacuate buffer for child process
            pid_t pid = fork(); // returns child pid

            if (pid > 0) { // parent process
                int status;
                waitpid(pid, &status, 0); // waits for terminating child process
                if (WIFEXITED(status)) {  // true if child terminated normally
                    lastStatus = WEXITSTATUS(status); // on success, returns exit status of child
                } else {
                    lastStatus = 1; // failure
                }
            } else if (pid == 0) { // child process
                execvp(argvArr[0], argvArr);
                fprintf(stderr, "%s: command not found\n", argvArr[0]); // if here, failure
                exit(1);
            } else {
                perror("fork");
                lastStatus = 1; // failure: parent failed to fork
            }
        }
    }

    return lastStatus;
}
