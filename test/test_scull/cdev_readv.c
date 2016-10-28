#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/uio.h>

#define DEVNAME "/dev/mycdev"
#define NUM 100

#define per(fmt,...) do { \
fprintf(stderr, fmt, ##__VA_ARGS__); \
exit(1); \
}while(0)

static void process(char *buf)
{
	int i;
	for (i = 0; i < NUM-1; i++)
		if (buf[i] == '\0')
			buf[i] = '-';
	buf[NUM - 1] = '\0';
}

int main(void)
{
	int fd, count, len;
	char buf1[] = "aabbccddeeffgghhiijjkkll\n";
	char buf2[] = "ooppqqrrss\n";
	char tmpbuf[NUM];
	char *ptr;
	struct iovec *pio, *rio;

	/*test trim every start*/
	if ((fd = open(DEVNAME, O_WRONLY)) == -1)
		per("can not open wronly\n");
	close(fd);

	memset(tmpbuf, 0, NUM);
	if ((fd = open(DEVNAME, O_RDWR)) == -1) 
		per("can not open %s\n", DEVNAME);
	
	if ((pio = malloc(2*sizeof(struct iovec))) == NULL)
		per("can not alloc struct iovec\n");
	
	pio[0].iov_base = buf1;
	pio[0].iov_len = sizeof(buf1);
	pio[1].iov_base = buf2;
	pio[1].iov_len = sizeof(buf2);
	len = writev(fd, pio , 2);
	if (len < 0)
		per("writev error\n");
	printf("written %d bytes\n", len);
	
	memset(pio, 0, sizeof(buf1) + sizeof(buf2));
	if (lseek(fd, 0, SEEK_SET) < 0)
		per("lseek error\n");
	pio[0].iov_base = tmpbuf;
	pio[0].iov_len = NUM;

	len = readv(fd, pio, 1);
	printf("read %d bytes\n", len);
	process(pio[0].iov_base);
	printf("iov1: %s\n", pio[0].iov_base);
	close(fd);
	return 0;
}
