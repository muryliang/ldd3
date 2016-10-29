#include "../head.h"
#include <signal.h>
#define DEVNAME "/dev/scullb"

static void handler(int sig)
{
	printf("wakeup in process %d\n", (int)getpid());
}
	

int main(void)
{
	int fd;

	if ((fd = open(DEVNAME, O_RDONLY)) < 0)
		per("error open\n");

	signal(SIGIO, handler);
	fcntl(fd, F_SETOWN, getpid());
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | FASYNC);

	printf("begin to sleep\n");
	while(1) {
		sleep(10);
	}
	return 0;
}

