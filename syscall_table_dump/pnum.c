#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
	printf("read=%u\n", 0xffffffff84280670);
	printf("write=%u\n", 0xffffffff84280730);
	printf("open=%u\n", 0xffffffff8427dac0);
	printf("creat=%u\n", 0xffffffff8427db20);
	return 0;
}
