#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define DEVNAME "/dev/mycdev"
#define NUM 50

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

	/*test trim every start*/
	if ((fd = open(DEVNAME, O_WRONLY)) == -1)
		per("can not open wronly\n");
	close(fd);

	memset(tmpbuf, 0, NUM);
	if ((fd = open(DEVNAME, O_RDWR)) == -1) 
		per("can not open %s\n", DEVNAME);
	

	if (lseek(fd, 42, SEEK_SET) == -1)
		per("can not seek over end\n");

	count = 0;
	ptr = buf1;
	while( count < strlen(buf1)+1) {
		len = write(fd, ptr, strlen(buf1)+1-count );
		if (len <=0)
			break;
		ptr += len;
		count += len;
	}
/*		
		
	if (write(fd, buf1, strlen(buf1)+1) < 0)
		per("error when write in\n");
*/

	if (lseek(fd, 578, SEEK_SET) == -1)
		per("can not seek over end\n");

	count = 0;
	ptr = buf2;
	while(count < strlen(buf2)+1) {
		len = write(fd, ptr, strlen(buf2)+1-count);
		if (len <=0)
			break;
		ptr += len;
		count += len;
	}
/*
	if (write(fd, buf2, strlen(buf2)+1) < 0)
		per("error when write in 2\n");
*/
	if (lseek(fd, 40, SEEK_SET) ==  -1) 
		per("error when lseek\n");
	memset(tmpbuf, 0, NUM);
	ptr = tmpbuf;
	count = 0;
	while((len = read(fd, ptr, NUM -1)) > 0) {
		count += len;
		if (count >= strlen(buf1) + 1)
			break;
		ptr += len;
	}
	process(tmpbuf);
	printf("recevice string count: %d\n"
		"%s", count, tmpbuf);
	putchar('\n');


	if (lseek(fd, 570, SEEK_SET) ==  -1) 
		per("error when lseek\n");
	memset(tmpbuf, 0, NUM);
	ptr = tmpbuf;
	count = 0;
	while ((len = read(fd, ptr, NUM -1)) > 0) {
		count += len;
		if (count >= strlen(buf2) + 1)
			break;
		ptr += len;
	}
	process(tmpbuf);
	printf("recevice string count: %d\n"
		"%s", count, tmpbuf);
	putchar('\n');

	close(fd);
	return 0;
}
