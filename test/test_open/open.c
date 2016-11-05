#include "../head.h"

int main(void)
{
	int  fd;
	char buf[8];

	if ((fd = open("/dev/sdev", O_RDWR)) == -1)
		per("can not open sdev\n");
	printf("open success\n");
	if (write(fd, "hahaha", 6) <= 0)
		per("write error\n");
	if (read (fd, buf, 6) != 6)
		per("read error\n");
	close(fd);
	printf("close success\n");
	return 0;
}

