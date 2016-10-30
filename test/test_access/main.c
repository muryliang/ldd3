#include "../head.h"

int main(void)
{
	int fd;

	if ((fd = open("/dev/scullb", O_RDWR)) < 0) {
		perror("why: ");
		per("can not open\n");
	}

	printf("success open\n");
	sleep(20);
	close(fd);
	return 0;
}
