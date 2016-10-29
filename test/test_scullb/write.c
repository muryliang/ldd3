#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

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

int main(int ac, char *av[])
{
	int fd, count = 1;
	char *def = "my name is bianmingliang";
	char *buf;

	if (ac == 1)
		buf = def;
	else
		buf = av[1];

	if ((fd = open(DEVNAME, O_WRONLY)) < 0)
		per("can not open %s\n", DEVNAME);

	count = write(fd, buf, strlen(buf) + 1);
	if (count <= 0 )
		per("can not write\n");
	printf("success written %d bytes\n", count);
	close(fd);
	return 0;
}

	
