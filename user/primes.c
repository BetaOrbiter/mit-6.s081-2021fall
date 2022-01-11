#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//wrapper for fork
//print stderr and exit when fail 
static inline int
Fork()
{
    int pid = fork();
    if(pid < 0){
        fprintf(2, "failed to create child process\n");
        exit(1);
    }
    return pid;
}

//wrapper for pipe
static inline int
Pipe(int *p)
{
    int ret = pipe(p);
    if(ret < 0){
        fprintf(2, "failed to create pipe\n");
        exit(1);
    }
    return ret;
}

//wrapper for write
static inline int
Write(int fd, const void *src, int size)
{
    int ret = write(fd, src, size);
    if(ret < 0){
        fprintf(2, "failed to write to pipe\n");
        exit(1);
    }
    return ret;
}

//wrapper for read
static inline int
Read(int fd, void *dst, int size)
{
    int ret = read(fd, dst, size);
    if(ret < 0){
        fprintf(2, "failed to read from pipe\n");
        exit(1);
    }
    return ret;
}

void
child_process(int pre_p[2]){
    close(pre_p[1]);
    int first;
    Read(pre_p[0],&first, sizeof(first));
    printf("prime %d\n", first);

    int flag,num;
    flag = Read(pre_p[0], &num, sizeof(num));
    if(0 != flag){
        int nxt_p[2];
        Pipe(nxt_p);
        if(0 == Fork()){
            close(pre_p[0]);
            child_process(nxt_p);
        }else{
            close(nxt_p[0]);
            
            do{
                if(0 != num%first)
                    Write(nxt_p[1], &num, sizeof(num));
            }while(0 != Read(pre_p[0], &num, sizeof(num)));
            
            close(pre_p[0]);
            close(nxt_p[1]);
            wait(0);
        }
    }
    exit(0);
}

int
main(void)
{
    int p[2];
    Pipe(p);
    if(0 == Fork()){
        child_process(p);
    }else{//first process
        close(p[0]);
        
        for(int n=2; n<=35; ++n)
            Write(p[1], &n, sizeof(n));
        
        close(p[1]);
        wait(0);
        exit(0);
    }
    return 0;
}