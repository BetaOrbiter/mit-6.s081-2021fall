#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int 
main(void)
{
    int p[2];
    int pid;

    pipe(p);
    pid = fork();
    if(pid == 0){
        char byte[1];
        pid = getpid();
        if(1 != read(p[0],byte,1)){
            fprintf(2,"child failed to recieve\n");
            exit(1);
        }
        close(p[0]);
        fprintf(0,"%d: received ping\n",pid);
        if(1 != write(p[1],byte,1)){
            fprintf(2,"child failed to send\n");
            exit(1);
        }
        close(p[1]);
        exit(0);
    }
    else{
        pid = getpid();
        char byte[1];
        if(1 != write(p[1],byte,1)){
            fprintf(2,"parent failed to send\n");
            exit(1);
        }
        close(p[1]);
        if(1 != read(p[0],byte,1)){
            fprintf(2,"parent failed to recieve\n");
            exit(1);
        }
        close(p[0]);
        fprintf(0,"%d: received pong\n",pid);
        exit(0);
    }
}