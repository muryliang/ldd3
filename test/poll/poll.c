#include "../head.h"

#include <sys/select.h>
#define DEVNAME "/dev/scullb"

int main(void)
{
	int fd, count;
	fd_set rset, wset, eset;
	char buf[BUFSIZ];

	if ((fd = open(DEVNAME, O_RDONLY)) < 0) 
		per("can not open\n");

	FD_ZERO(&rset);
	FD_ZERO(&wset);
	FD_ZERO(&eset);
	FD_SET(fd, &rset);
	FD_SET(fd, &wset);
	FD_SET(fd, &eset);
	printf("begin to select\n");
	if (select(fd+1, &rset, &wset, &eset, NULL) < 0)
		per("select failed\n");
	if (FD_ISSET(fd, &rset)) {
		count = read(fd, buf, BUFSIZ);
		if (count > 0) {
			buf[count] = '\0';
			printf("string get:\n%s\n", buf);
		}
	}
	if (FD_ISSET(fd, &wset)) 
		printf("poll in write\n");
	if (FD_ISSET(fd, &eset)) 
		printf("poll in except\n");
	return 0;
}

