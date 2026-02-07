#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int femtoshell_main(int argc, char *argv[]) {

    char *command = NULL;
    size_t bufsize = 0;
    int status = 0;

    while (1) {
        printf("My simple shelll > \n");

        ssize_t nread = getline(&command, &bufsize, stdin);
        if (nread == -1) {
            printf("\n");
            break;
        }

        if (nread > 0 && command[nread - 1] == '\n') {
            command[nread - 1] = '\0';
            nread--;
        }

        if (nread == 0) {
            printf("\n");
            continue;
        }

        if (strncmp(command, "echo", 4) == 0) {
            char *text = command + 4;
            if (*text == ' ')
                text++;
            printf("\n%s\n", text);
        }

        else if (strcmp(command, "exit") == 0) {
            printf("\nGood Bye\n");
            break;
        }

        else {
            printf("\nInvalid command\n");
            status = 1;
        }
    }

    return status;
}

