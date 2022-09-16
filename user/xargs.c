#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define BUFSIZE 1000

char* readline(char* buf){
    static int i = 0;
    
    if(i >= strlen(buf) || buf[i] == '\0')  return 0;
    else if(buf[i] == '\n') i++;

    char* rec = "", *h = rec; *rec = '\0';
    printf("begin-i:%d, h:%s, rec:%s, rec-char:%d\n, ", i, h, rec, *rec == '\0');
    while(i < strlen(buf) && buf[i] != '\n')   *rec++ = buf[i++];
    *rec = '\0'; 
    printf("i: %d, len: %d, str: %s, end-char: %d\n", i, strlen(buf), h, buf[i] == '\n');
    return h;
}
int 
main(int argc, char *argv[])
{
    char buf[BUFSIZE];
    read(0, buf, BUFSIZE);
    printf("标准化输入： %s, len: %d\n", buf, strlen(buf));
    char* args[argc - 1];
    for(int i = 0; i < argc - 1; i++)
        args[i] = argv[i + 1];
    for(int i = 0; i < argc - 1; i++){
        printf("参数: %s\n", args[i]);
    }

    char* get;
    int j = 0;
    while((get = readline(buf)) != 0){
        j++;
        if(j > 5)   break;
        // printf("get: %s\n", get);
    }
    exit(0); 
}



