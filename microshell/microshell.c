#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#define BUF_SIZE 65536

const char *PROMPT = "Nano Shell Prompt >\n";
int last_status = 0;
int varCount = 0;

typedef struct {
    char *name;
    char *value;
    int exported;
} ShellVar;

typedef struct {
    int type;
    char *file;
} Redirection;

ShellVar *variables = NULL;

ShellVar* find_variable(const char *name) {
    for (int i = 0; i < varCount; i++) {
        if (strcmp(variables[i].name, name) == 0)
            return &variables[i];
    }
    return NULL;
}

void set_variable(const char *name, const char *value) {
    ShellVar *var = find_variable(name);

    if (var) {
        free(var->value);
        var->value = strdup(value);
        return;
    }

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
    setenv(var->name, var->value, 1);
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

int microshell_main(int argc, char *argv[]) {

    char buf[BUF_SIZE];

    while (1) {
        printf(PROMPT);
        fflush(stdout);

        if (!fgets(buf, BUF_SIZE, stdin))
            break;

        buf[strcspn(buf, "\n")] = 0;

        if (strlen(buf) == 0)
            continue;

        char *equal = strchr(buf, '=');
        if (equal && !strchr(buf, ' ')) {
            *equal = '\0';
            char *name = buf;
            char *value = equal + 1;

            if (strlen(name) > 0) {
                set_variable(name, value);
                last_status = 0;
                continue;
            }
        }

        char *raw_tokens[4096];
        int raw_count = 0;

        char *token = strtok(buf, " ");
        while (token) {
            raw_tokens[raw_count++] = strdup(token);
            token = strtok(NULL, " ");
        }

        if (raw_count == 0)
            continue;

        char *tokens[4096];
        for (int i = 0; i < raw_count; i++)
            tokens[i] = substitute_string(raw_tokens[i]);

        tokens[raw_count] = NULL;

        Redirection redirs[100];
        int redir_count = 0;

        char *argv_exec[4096];
        int argc_exec = 0;

        for (int i = 0; i < raw_count; i++) {

            if (strcmp(tokens[i], "<") == 0 && i + 1 < raw_count) {
                redirs[redir_count].type = 0;
                redirs[redir_count++].file = tokens[++i];
            }
            else if (strcmp(tokens[i], ">") == 0 && i + 1 < raw_count) {
                redirs[redir_count].type = 1;
                redirs[redir_count++].file = tokens[++i];
            }
            else if (strcmp(tokens[i], "2>") == 0 && i + 1 < raw_count) {
                redirs[redir_count].type = 2;
                redirs[redir_count++].file = tokens[++i];
            }
            else {
                argv_exec[argc_exec++] = tokens[i];
            }
        }

        argv_exec[argc_exec] = NULL;

        if (argc_exec == 0)
            continue;
int redir_failed = 0;

if (
    strcmp(argv_exec[0], "echo") == 0 ||
    strcmp(argv_exec[0], "pwd") == 0 ||
    strcmp(argv_exec[0], "cd") == 0 ||
    strcmp(argv_exec[0], "export") == 0
) {
    for (int i = 0; i < redir_count; i++) {

        int fd;

        if (redirs[i].type == 0) {
            fd = open(redirs[i].file, O_RDONLY);
        } else {
            fd = open(redirs[i].file,
                      O_WRONLY | O_CREAT | O_TRUNC,
                      0644);
        }

        if (fd < 0) {
            fprintf(stderr, "%s: %s\n",
                    redirs[i].file, strerror(errno));
            last_status = 1;
            redir_failed = 1;
            break;
        }

        close(fd);
    }

    if (redir_failed)
        continue;
}

        if (strcmp(argv_exec[0], "exit") == 0) {
            printf("Good Bye\n");
            return last_status;
        }

        if (strcmp(argv_exec[0], "cd") == 0) {
            if (!argv_exec[1] || chdir(argv_exec[1]) != 0) {
                printf("cd: %s: No such file or directory\n",
                       argv_exec[1] ? argv_exec[1] : "");
                last_status = 1;
            } else {
                last_status = 0;
            }
            continue;
        }

        if (strcmp(argv_exec[0], "pwd") == 0) {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd))) {
                printf("%s\n", cwd);
                last_status = 0;
            }
            continue;
        }

        if (strcmp(argv_exec[0], "export") == 0) {
            if (argv_exec[1])
                export_variable(argv_exec[1]);
            last_status = 0;
            continue;
        }

        if (strcmp(argv_exec[0], "echo") == 0) {
            for (int i = 1; argv_exec[i]; i++) {
                printf("%s", argv_exec[i]);
                if (argv_exec[i+1])
                    printf(" ");
            }
            printf("\n");
            last_status = 0;
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {

            for (int i = 0; i < redir_count; i++) {

                int fd;

                if (redirs[i].type == 0) {
                    fd = open(redirs[i].file, O_RDONLY);
                    if (fd < 0) {
                       fprintf(stderr, "cannot access %s: %s\n",
        redirs[i].file, strerror(errno));
                        exit(1);
                    }
                    dup2(fd, STDIN_FILENO);
                    close(fd);
                }

                else if (redirs[i].type == 1) {
                    fd = open(redirs[i].file,
                              O_WRONLY | O_CREAT | O_TRUNC,
                              0644);
                    if (fd < 0) {
                        fprintf(stderr, "%s: %s\n",
        redirs[i].file, strerror(errno));
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    close(fd);
                }

                else if (redirs[i].type == 2) {
                    fd = open(redirs[i].file,
                              O_WRONLY | O_CREAT | O_TRUNC,
                              0644);
                    if (fd < 0) {
                        fprintf(stderr, "%s: %s\n",
                               redirs[i].file, strerror(errno));
                        exit(1);
                    }
                    dup2(fd, STDERR_FILENO);
                    close(fd);
                }
            }

            execvp(argv_exec[0], argv_exec);
            fprintf(stderr, "%s: command not found\n", argv_exec[0]);
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
