#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define DEVNAME "/dev/mycdev"
#define NUM 100

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

	if ((fd = open(DEVNAME, O_RDWR)) == -1) {
		per("can not open %s\n", DEVNAME);
	}
	printf("success open\n");

	printf("about to write\n");
	if (write(fd, buf1, strlen(buf1)+1) < 0)
		per("error when write in\n");

	if (write(fd, buf2, strlen(buf2)+1) < 0)
		per("error when write in 2\n");

	errno = 0;
	printf("about to seek\n");
	if (lseek(fd, 0, SEEK_SET) ==  -1 && errno != 0) {
		perror("why\n");
		per("error when lseek\n");
	}
	
	printf("about to read\n");
	count = read(fd, tmpbuf, NUM -1);
	if (count > 0) {
		tmpbuf[count] = '\0';
		process(tmpbuf);
		printf("recevice string count: %d\n"
			"%s", count, tmpbuf);
	}
	printf("about to close\n");	
	close(fd);
	return 0;
}
