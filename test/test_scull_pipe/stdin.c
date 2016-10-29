#include "head.h"

int main(void)
{
	int delay = 1, n, m = 0;
	char buffer[BUFSIZ];
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
	fcntl(1, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);

	while(1) {
		n = read(0, buffer, BUFSIZ);
		if (n >= 0)
			m = write(1, buffer, n);
		if ((n < 0 || m < 0) && (errno != EAGAIN) )
			break;
	}
	perror(n < 0 ? "stdin" : "stdout");
	exit(1);
}
