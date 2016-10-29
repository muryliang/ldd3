#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define DEVNAME "/dev/scullb"
#define SIZE 50


#define per(fmt, ...) do {  \
printf(fmt, ##__VA_ARGS__); \
exit(1); \
} while(0)

static int process(char *buf)
{
	int i;
	for (i = 0; i < SIZE-1; i++)
		if (buf[i] == '\0')
			buf[i] = '-';
}

int main(void)
{
	int fd, count = 1;
	char buf[SIZE];

	if ((fd = open(DEVNAME, O_RDONLY)) < 0)
		per("can not open %s\n", DEVNAME);

	while (count > 0) {
		count = read(fd, buf, SIZE - 1);
		if (count <= 0 )
			break;
		buf[count] = '\0';
		buf[SIZE-1] = '\0';
	//	process(buf);
		printf("get %d bytes\n"
			"string:%s", count, buf);
		putchar('\n');
		fflush(stdout);
	}

	printf("done\n");
	close(fd);
	return 0;
}

	
