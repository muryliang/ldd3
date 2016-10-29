#include "head.h"

#define DEVNAME "/dev/scullb"
int main(void)
{
	int fd, count;
	char buf[1];

	if ( (fd = open(DEVNAME, O_RDONLY)) < 0)
		per("can not open for read\n");

	while((count = read(fd, buf, 1)) >=0 ) {
		printf("read\n");
		sleep(1);
	}
	
	printf("close\n");
	close(fd);
	return 0;
}
	
