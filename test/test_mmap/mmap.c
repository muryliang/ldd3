#include "../head.h"

int main(void)
{
	int fd, i;
	char *buf;

	if ((fd = open("/dev/sdev", O_RDWR)) < 0)
		per("error open dev\n");

	errno = 0;
	buf = mmap(NULL, 40, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (buf == MAP_FAILED) {
		perror("error mmap: ");
		per("failed\n");
	}

	printf("mapped success begin write mapped conten\n");

	for(i = 0; i < 40; i++) {
		if ( i < 25)
			buf[i] = 'a' + i;
		else
			buf[i] = 'A' + i - 25;
	}
	printf("begin read\n");

	for (i = 0; i < 40; i++)
		putchar(buf[i]);
	putchar('\n');

	if (munmap(buf, 40) < 0)
		per("error unmap\n");
	printf("unmap success\n");

	return 0;
}

	
