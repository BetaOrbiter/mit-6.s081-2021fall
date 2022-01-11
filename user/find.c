#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

static void
find(const char *path, const char *name)
{
    char buf[512], *p;
    int dir_fd;
    struct dirent de;
    struct stat st;

    if((dir_fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n",path);
        return;
    }
    if(fstat(dir_fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(dir_fd);
        return;
    }
    if(st.type != T_DIR){
        fprintf(2, "Usage: find dir file\n");
        close(dir_fd);
        return;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    while(read(dir_fd, &de, sizeof(de)) == sizeof(de)){
        if(de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0){
            printf("find: cannot stat %s\n", buf);
            continue;
        }
        if(st.type == T_DIR)
            find(buf, name);
        else if(st.type == T_FILE)
                if(0 == strcmp(de.name, name))
                    printf("%s\n", buf);
    }
    close(dir_fd);
}  

int
main(int argc, char *argv[]){
    if(argc != 3){
        fprintf(2, "Usage: find dir file\n");
        exit(1);
    }

    find(argv[1], argv[2]);
    exit(0);
}