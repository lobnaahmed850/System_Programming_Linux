#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

int cp_main(int argc, char *argv[]) {

    int inputFd, outputFd;
    int numRead;
    char buffer[BUFF_SIZE];

    if(argc !=3 ) {
         printf("Usage: cp <source_file> <destination_file>\n");
         exit(-1);
    }
    inputFd = open(argv[1],O_RDONLY);
    if(inputFd<0) {
        printf("Can't open source file\n");
        exit(-2);
    }
    outputFd = open(argv[2],O_WRONLY | O_CREAT | O_TRUNC,0644);
    if(outputFd<0) {
        printf("Can't open destination file\n");
        close(inputFd);
        exit(-3);
    }
    while((numRead=read(inputFd,buffer,BUFF_SIZE)) > 0) {
        if(write(outputFd,buffer,numRead) != numRead) {
            printf("Write failed\n");
            close(inputFd);
            close(outputFd);
            exit(-4);
        }
    }
    if(numRead<0) {
        printf("Read failed\n");
        exit(-5);
    }
    close(inputFd);
    close(outputFd);

    return 0;

}
