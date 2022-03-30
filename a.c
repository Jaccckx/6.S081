#include <stdio.h>

void p(int*p)	{
//	printf("p: %d\n", *p);
	return;
}

int main(){
	int a = 1;
	p((int *)a);
}
