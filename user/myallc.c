#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(){
	char* m1 = (char*)myalloc(2 * 4096);
	char* m2 = (char*)myalloc(3 * 4096);
	char* m3 = (char*)myalloc(1 * 4096);
	char* m4 = (char*)myalloc(7 * 4096);
	char* m5 = (char*)myalloc(9 * 4096);

	// m1[0] = 'h';
	// m1[1] = '\0';
	// printf("1: m1: %s\n", m1);

	myfree((uint64)m2);


	// m2[0] = 'h';
	// m2[1] = '\0';
	// printf("1: free m1: %s\n", m2);


	myfree((uint64)m4);

	// sleep(5000);
	myfree((uint64)m1);
	myfree((uint64)m3);
	myfree((uint64)m5);
	printf("myallc test is passed\n");
	exit(0);
}
	
