//
// tests for copy-on-write fork() assignment.
//

#include "kernel/types.h"
#include "kernel/memlayout.h"
#include "user/user.h"
// allocate more than half of physical memory,
// then fork. this will fail in the default
// kernel, which does not support copy-on-write.
void
simpletest()
{
  uint64 phys_size = PHYSTOP - KERNBASE;
  int sz = (phys_size / 3) * 2;

  printf("simple: ");

  char *p = sbrk(sz);
  if(p == (char*)0xffffffffffffffffL){
    printf("sbrk(%d) failed\n", sz);
    exit(-1);
  }

  for(char *q = p; q < p + sz; q += 4096){
    *(int*)q = getpid();
  }
  // exit(0);
  int pid = fork();
  if(pid < 0){
    printf("fork() failed\n");
    exit(-1);
  }

  if(pid == 0)
    exit(0);

  wait(0);

  if(sbrk(-sz) == (char*)0xffffffffffffffffL){
    printf("sbrk(-%d) failed\n", sz);
    exit(-1);
  }

  printf("ok\n");
}

// three processes all write COW memory.
// this causes more than half of physical memory
// to be allocated, so it also checks whether
// copied pages are freed.
void
threetest()
{
  uint64 phys_size = PHYSTOP - KERNBASE;
  int sz = phys_size / 4;
  int pid1, pid2;

  printf("three: ");
  
  char *p = sbrk(sz);
  if(p == (char*)0xffffffffffffffffL){
    printf("sbrk(%d) failed\n", sz);
    exit(-1);
  }

  pid1 = fork();
  if(pid1 < 0){
    printf("fork failed\n");
    exit(-1);
  }
  if(pid1 == 0){
    pid2 = fork();
    if(pid2 < 0){
      printf("fork failed");
      exit(-1);
    }
    if(pid2 == 0){
      for(char *q = p; q < p + (sz/5)*4; q += 4096){
        *(int*)q = getpid();
      }
      for(char *q = p; q < p + (sz/5)*4; q += 4096){
        if(*(int*)q != getpid()){
          printf("wrong content\n");
          exit(-1);
        }
      }
      exit(-1);
    }
    for(char *q = p; q < p + (sz/2); q += 4096){
      *(int*)q = 9999;
    }
    exit(0);
  }

  for(char *q = p; q < p + sz; q += 4096){
    *(int*)q = getpid();
  }

  wait(0);

  sleep(1);

  for(char *q = p; q < p + sz; q += 4096){
    if(*(int*)q != getpid()){
      printf("wrong content\n");
      exit(-1);
    }
  }

  if(sbrk(-sz) == (char*)0xffffffffffffffffL){
    printf("sbrk(-%d) failed\n", sz);
    exit(-1);
  }

  printf("ok\n");
}

char junk1[4096];
int fds[2];
char junk2[4096];
char buf[4096];
char junk3[4096];

// test whether copyout() simulates COW faults.
void
filetest()
{
  printf("file: ");
  
  buf[0] = 99;

  for(int i = 0; i < 4; i++){
    if(pipe(fds) != 0){
      printf("pipe() failed\n");
      exit(-1);
    }
    // i = 50;
    int pid = fork();
    if(pid < 0){
      printf("fork failed\n");
      exit(-1);
    }
    if(pid == 0){
      sleep(1);
      if(read(fds[0], buf, sizeof(i)) != sizeof(i)){
        printf("error: read failed\n");
        exit(1);
      }
      sleep(1);
      int j = *(int*)buf;
      if(j != i){
        // printf("i:%d, j:%d\n", i,j);
        printf("error: read the wrong value\n");
        exit(-1);
      }
      exit(0);
    }
    // i = 200;
    if(write(fds[1], &i, sizeof(i)) != sizeof(i)){
      printf("error: write failed\n");
      exit(-1);
    }
  }

  int xstatus = 0;
  for(int i = 0; i < 4; i++) {
    wait(&xstatus);
    if(xstatus != 0) {
      exit(1);
    }
  }

  if(buf[0] != 99){
    printf("error: child overwrote parent\n");
    exit(1);
  }

  printf("ok\n");
}

int
main(int argc, char *argv[])
{
  // exit(0);
  simpletest();
  // exit(0);
  // check that the first simpletest() freed the physical memory.
  simpletest();
  // exit(0);
  threetest();
  threetest();
  threetest();

  filetest();

  printf("ALL COW TESTS PASSED\n");

  exit(0);
}
// #define O_RDONLY  0x000
// #define O_WRONLY  0x001
// #define O_RDWR    0x002
// #define O_CREATE  0x200
// #define O_TRUNC   0x400

// void
// copyin(char *s)
// {
//   uint64 addrs[] = { 0x80000000LL, 0xffffffffffffffff };

//   for(int ai = 0; ai < 2; ai++){
//     uint64 addr = addrs[ai];
    
//     int fd = open("copyin1", O_CREATE|O_WRONLY);
//     if(fd < 0){
//       printf("open(copyin1) failed\n");
//       exit(1);
//     }
//     int n = write(fd, (void*)addr, 8192);
//     if(n >= 0){
//       printf("write(fd, %p, 8192) returned %d, not -1\n", addr, n);
//       exit(1);
//     }
//     close(fd);
//     unlink("copyin1");
    
//     n = write(1, (char*)addr, 8192);
//     if(n > 0){
//       printf("write(1, %p, 8192) returned %d, not -1 or 0\n", addr, n);
//       exit(1);
//     }
//     printf("ok\n");   
//     int fds[2];
//     if(pipe(fds) < 0){
//       printf("pipe() failed\n");
//       exit(1);
//     }
//     n = write(fds[1], (char*)addr, 8192);
//     if(n > 0){
//       printf("write(pipe, %p, 8192) returned %d, not -1 or 0\n", addr, n);
//       exit(1);
//     }
//     close(fds[0]);
//     close(fds[1]);
//   }
// }

// void
// copyout(char *s)
// {
//   uint64 addrs[] = { 0x80000000LL, 0xffffffffffffffff };
//   // printf("copy out\n");
//   for(int ai = 0; ai < 2; ai++){
//     uint64 addr = addrs[ai];

//     int fd = open("README", 0);
//     if(fd < 0){
//       printf("open(README) failed\n");
//       exit(1);
//     }
//     int n = read(fd, (void*)addr, 8192);
//     if(n > 0){
//       printf("read(fd, %p, 8192) returned %d, not -1 or 0\n", addr, n);
//       exit(1);
//     }
//     close(fd);

//     int fds[2];
//     if(pipe(fds) < 0){
//       printf("pipe() failed\n");
//       exit(1);
//     }
//     n = write(fds[1], "x", 1);
//     if(n != 1){
//       printf("pipe write failed\n");
//       exit(1);
//     }
//     n = read(fds[0], (void*)addr, 8192);
//     if(n > 0){
//       printf("read(pipe, %p, 8192) returned %d, not -1 or 0\n", addr, n);
//       exit(1);
//     }
//     close(fds[0]);
//     close(fds[1]);
//   }
// }

// int main(){
//   // copyin("copyin");
//   // printf("ok1\n");
//   copyout("copyout");
//   printf("ok copyout\n");
//   exit(0);
// }