#include "head.h"

#define DEVNAME "/dev/scullb"
int main(void)
{
	int fd, count;
	char buf[1];

	if ( (fd = open(DEVNAME, O_WRONLY)) < 0)
		per("can not open for write\n");

	while((count = write(fd, buf, 1)) >=0 ) {
		printf("write\n");
		sleep(1);
	}
	
	printf("close\n");
	close(fd);
	return 0;
}
	
