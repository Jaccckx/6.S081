#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int fd[2];
  pipe(fd);
  if(fork() == 0){
    close(fd[0]);
    write(fd[1], " ", 1);
    printf("%d: received ping\n", getpid());
    close(fd[1]); 
  }else{
    wait(0);
    close(fd[1]);
    read(fd[0], " ", 1);
    printf("%d: received pong\n", getpid());
    close(fd[0]);
  }
  exit(0);
}
