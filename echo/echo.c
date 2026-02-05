 #include <string.h>
 #include <stdio.h>
 #include <unistd.h>

int echo_main(int argc, char *argv[]) {

    for(int i=1;i<argc;i++) {
       if (write(1,argv[i],strlen(argv[i])) <0) {
           printf("Write failed\n");
           exit(-1);
       }
       if(i<argc-1) {
           if(write(1," ",1) <0)
           exit(-2);
       }
    }
    if(write(1,"\n",1) <0)
    exit(-3);

    return 0;
}
