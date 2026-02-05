#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>

int pwd_main() {
    char buffer[PATH_MAX];

    if (getcwd(buffer, sizeof(buffer)) == NULL) {
       printf("can't get current directory\n");
    exit(-1);
    }

    printf("%s\n", buffer);

    return 0;
}
