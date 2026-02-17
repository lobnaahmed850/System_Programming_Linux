#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>

#define BUF_SIZE 65536

const char *PROMPT = "Nano Shell Prompt >\n";
int last_status = 0;
int varCount = 0;

typedef struct {
    char *name;
    char *value;
    int exported;
} ShellVar;

ShellVar *variables = NULL; //dynamic array of variables

ShellVar* find_variable(const char *name) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(variables[i].name, name) == 0)
            return &variables[i]; //reuse var
    }
    return NULL; //var not found
}

void set_variable(const char *name, const char *value) {
    ShellVar *var = find_variable(name);

    if (var) { //var found -> update value -> terminate func
        free(var->value);
        var->value = strdup(value);
        return;
    }
    //var not found -> increase arr size -> store name & value
    variables = (ShellVar*)realloc(variables, (varCount + 1) * sizeof(ShellVar));
    variables[varCount].name = strdup(name);
    variables[varCount].value = strdup(value);
    variables[varCount].exported = 0;
    varCount++;
}

void export_variable(const char *name) {
    ShellVar *var = find_variable(name);
    if (!var) return;

    var->exported = 1;
    setenv(var->name, var->value, 1); //add to linux env, childs can inherit
}

char* substitute_string(const char *input) {
    char result[BUF_SIZE] = {0};
    while (*input) {
        if (*input == '$') {
            input++;
            char varname[256] = {0};
            int j = 0;

            while (*input && (isalnum(*input) || *input == '_')) {
                varname[j++] = *input++;
            }

            ShellVar *var = find_variable(varname);
            char *value = var ? var->value : getenv(varname);
            if (value)
                strcat(result, value);
        } else {
            int len = strlen(result);
            result[len] = *input;
            result[len+1] = '\0';
            input++;
        }
    }

    return strdup(result);
}


int nanoshell_main(int argc, char *argv[]) {

    char buf[BUF_SIZE];

    while (1) {
        printf(PROMPT);
        fflush(stdout);

        if (!fgets(buf, BUF_SIZE, stdin))
            break;

        buf[strlen(buf) - 1] = 0;
        if (strlen(buf) == 0)
            continue;

            //handling assignment
        char *equal = strchr(buf, '=');
        if (equal && !strchr(buf, ' ')) {
            *equal = '\0';
            char *name = buf;
            char *value = equal + 1;

            if (strlen(name) > 0 && strlen(value) > 0) {
                set_variable(name, value);
                last_status = 0;
                continue;
            }
        }

        //tokenization command into arguments
        char *tokens[4096];
        int count = 0;

        char *token = strtok(buf, " ");
        while (token) {
            tokens[count++] = substitute_string(token);
            token = strtok(NULL, " ");
        }
        tokens[count] = NULL;

        if (count == 0)
            continue;

            //exit
        if (strcmp(tokens[0], "exit") == 0) {
            printf("Good Bye\n");
            return last_status;
        }

        //cd
        if (strcmp(tokens[0], "cd") == 0) {
            if (tokens[1] == NULL || chdir(tokens[1]) != 0) {
                printf("cd: %s: No such file or directory\n", tokens[1]);
                last_status = 1;
            }
            else
                last_status = 0;
            continue;
        }

        //pwd
        if (strcmp(tokens[0], "pwd") == 0) {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd))) {
                printf("%s\n", cwd);
                last_status = 0;
            }
            continue;
        }

        //export
        if (strcmp(tokens[0], "export") == 0) {
            if (tokens[1])
                export_variable(tokens[1]);
            last_status = 0;
            continue;
        }

        //external commands
        pid_t pid = fork();
        if (pid == 0) {
            execvp(tokens[0], tokens);
            printf("%s: command not found\n", tokens[0]);
            exit(1);
        }
        else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
            last_status = WEXITSTATUS(status);
        }
        else {
            perror("fork");
            last_status = 1;
        }
    }

    return last_status;
}

