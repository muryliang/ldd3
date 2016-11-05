#include "../head.h"

#define DEVNAME "/dev/sdev"

int main(void)
{
	int fd, count, size = 0;
	char buf[] = "hahahe";
	char *ptr = buf;

	if ((fd = open(DEVNAME, O_WRONLY)) <0)
		per("error on open dev for write\n");
	
	do { 
		count = write(fd, ptr, strlen(ptr));
		size += count;
		ptr += count;
		printf("sending... %d\n",count);
	} while (size < strlen(buf));
	close(fd);

	return 0;
}

