#include <sys/select.h>

#include <limits.h>
#include <stdio.h>

#include "sad.h"

static int
wavinit(void)
{
	return 0;
}

static int
wavopen(int fd)
{
	return 0;
}

static int
wavdecode(int fd)
{
	return 0;
}

static int
wavclose(void)
{
	return 0;
}

static void
wavexit(void)
{
}

Decoder wavdecoder = {
	.init = wavinit,
	.open = wavopen,
	.decode = wavdecode,
	.close = wavclose,
	.exit = wavexit
};
