#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    if(argc != 2){
        char* err = "sleep para err, plz input time\n";
        write(1, err, strlen(err));
    }
    int i = atoi(argv[1]);
    sleep(i);
    exit(0);
}
