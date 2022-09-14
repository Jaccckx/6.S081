#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char* argv[]){
    if(argc == 1){
        char* err = "sleep para error,plz input time\n";
        write(1, err, strlen(err));
    }
    int t = atoi(argv[1]);
    sleep(t);
    exit(0);
}