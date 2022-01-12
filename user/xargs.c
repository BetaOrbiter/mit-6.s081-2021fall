#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

static inline int
Read(int fd, void *dst, int size)
{
    int ret = read(fd, dst, size);
    if(ret < 0){
        fprintf(2,"failed to read\n");
        exit(1);
    }
    return ret;
}

static int
read_line(char *buf, const int max_size)
{
    int i;
    for(i=0;i<max_size;++i){
        if(0 == Read(0, &buf[i], sizeof(buf[i])))
            break;
        if('\n' == buf[i])
            break;
    }
    if(max_size == i){
        fprintf(2, "command line too long\n");
        exit(1);
    }
    buf[i] = '\0';
    return i;
}

static int
parse_line(char *line, char *argv[], int argc_existing)
{
    char *p=line;
    while(argc_existing < MAXARG){
        if(*p == ' ')
            *p++ = '\0';
        else if(*p == '\0')
                break;
        else {
            argv[argc_existing++] = p++;
            //slide over the argument
            while(*p != '\0' && *p != ' ')
                ++p;
        } 
    }
    if(MAXARG == argc_existing){
        fprintf(2, "Too many arguments\n");
        exit(1);
    }
    return argc_existing;
}
int
main(int argc, char *argv[]){
    if(argc < 2){
        fprintf(2, "Usage: xargs [command [initial-arguments]\n");
        exit(1);
    }
    
    char buf[1024];
    char *exec_argv[MAXARG+1];
    for(int i=0;i+1<argc;++i)
        exec_argv[i] = argv[i+1];

    while(read_line(buf, 1024)){
        int exec_argc = parse_line(buf, exec_argv, argc-1);
        exec_argv[exec_argc] = 0;
        if(fork() == 0){
            exec(argv[1], exec_argv);
            fprintf(2, "exec failed\n");
            exit(1);
        }
        wait(0);
    }
    return 0;
}