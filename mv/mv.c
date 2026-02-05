#include <stdio.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFF_SIZE 1024

int mv_main(int argc, char *argv[]) {

    int srcFd,dstFd,numRead;
    char buff[BUFF_SIZE];

   if(argc != 3) {
       printf("Usage: mv <source> <destination>\n");
       exit(-1);
   }
   int stat = rename(argv[1],argv[2]);
   if(stat==0) {
       return 0;
   }
   if((srcFd=open(argv[1],O_RDONLY))<0) {
       printf("Can't open source file\n");
       exit(-2);
   }
   if((dstFd=open(argv[2],O_WRONLY | O_CREAT | O_TRUNC,0644)) < 0) {
       printf("Can' open destination file\n");
       close(srcFd);
       exit(-3);
   }
   while(numRead=read(srcFd,buff,BUFF_SIZE) > 0) {
       if(write(dstFd,buff,numRead)!=numRead) {
           printf("Write failed\n");
           close(srcFd);
           close(dstFd);
           exit(-4);
       }
   }
   if(numRead<0) {
       printf("Read failed\n");
           close(srcFd);
           close(dstFd);
           exit(-5);
   }
   close(srcFd);
   close(dstFd);

   if(unlink(argv[1])<0) {
       printf("Can't remove source file\n");
       exit(-6);
   }

   return 0;

}
