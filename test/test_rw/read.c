#include "../head.h"

#define DEVNAME "/dev/sdev"

int main(void)
{
	int fd, count;
	char buf[50];

	if ((fd = open(DEVNAME, O_RDONLY)) <0)
		per("error on open dev for read\n");
	
	while( (count = read(fd, buf, 49)) > 0) {
		buf[count] = '\0';
		printf("recv string:\n%s\n");
	}
	close(fd);
	printf("bye\n");
	return 0;
}

