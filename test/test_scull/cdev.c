#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define DEVNAME "/dev/mycdev"

int main(void)
{
	int fd;

	if ((fd = open(DEVNAME, O_RDWR)) == -1) {
		printf("error when open\n");
		exit(1);
	}

	printf("success open\n");

	close(fd);
	return 0;
}
