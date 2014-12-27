#include <sys/select.h>

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

#include "sad.h"

static int fifofd = -1;

static int
fifovol(int vol)
{
	return 0;
}

static int
fifoopen(int bits, int rate, int channels)
{
	unlink("/tmp/sad-fifo");
	if (mkfifo("/tmp/sad-fifo", 0644) < 0) {
		warn("mkfifo /tmp/sad-fifo");
		return -1;
	}

	fifofd = open("/tmp/sad-fifo", O_RDWR | O_NONBLOCK);
	if (fifofd < 0) {
		warn("open /tmp/sad-fifo");
		return -1;
	}
	return 0;
}

static int
fifoplay(void *buf, size_t nbytes)
{
	ssize_t n, wrote;
	char    *p = buf;

	wrote = 0;
	while (nbytes > 0) {
		n = write(fifofd, &p[wrote], nbytes);
		if (n < 0)
			return nbytes; /* ignore errors */
		else if (n == 0)
			break;
		wrote += n;
		nbytes -= n;
	}
	return wrote;
}

static int
fifoclose(void)
{
	if (fifofd != -1) {
		close(fifofd);
		fifofd = -1;
	}
	return 0;
}

Output fifooutput = {
	.vol = fifovol,
	.open = fifoopen,
	.play = fifoplay,
	.close = fifoclose,
};
