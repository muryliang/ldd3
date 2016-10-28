#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>

typedef unsigned long long u64;
#define DEVNAME "/dev/mycdev"
#define NUM 100
#define GETINFO _IOR(0x20, 0, u64)

#define per(fmt,...) do { \
fprintf(stderr, fmt, ##__VA_ARGS__); \
exit(1); \
}while(0)

static void process(char *buf)
{
	int i;
	for (i = 0; i < NUM; i++)
		if (buf[i] == '\0')
			buf[i] = '-';
}

int main(void)
{
	int fd, count;
	char buf1[] = "my name is bianmingliang\n";
	char buf2[] = "now second\n";
	char tmpbuf[NUM];
	unsigned long long var = 0;

	if ((fd = open(DEVNAME, O_RDWR)) == -1) {
		per("can not open %s\n", DEVNAME);
	}
	printf("success open\n");

	if (ioctl(fd, GETINFO, (unsigned long)&var) < 0) {
		perror("");
		per("ioctl failed\n");
	}
	printf("got value var: %llu\n", var);
	close(fd);
	return 0;
}
