#include <sys/select.h>

#include <limits.h>
#include <stdio.h>

#include <mpg123.h>

#include "sad.h"

static mpg123_handle *handle;

static int
mp3init(void)
{
	mpg123_init();
	handle = mpg123_new(NULL, NULL);
	if (!handle)
		return -1;
	return 0;
}

static int
mp3open(const char *path)
{
	mpg123_open(handle, path) != MPG123_OK ? -1 : 0;
}

static size_t
mp3bufsz(void)
{
	return mpg123_outblock(handle);
}

static int
mp3getfmt(long *rate, int *channels, int *bits)
{
	long r;
	int  c, e;
	int  ret;

	ret = mpg123_getformat(handle, &r, &c, &e);
	if (ret != MPG123_OK)
		return -1;
	*rate = r;
	*channels = c;
	*bits = mpg123_encsize(e) * 8;
	return 0;
}

static int
mp3read(void *buf, size_t size)
{
	size_t done;
	int    r;

	r = mpg123_read(handle, buf, size, &done);
	if (r != MPG123_OK)
		return -1;
	return done;
}

static int
mp3close(void)
{
	return mpg123_close(handle) != MPG123_OK ? -1 : 0;
}

static void
mp3exit(void)
{
	mpg123_delete(handle);
	mpg123_exit();
}

Decoder mp3decoder = {
	.init = mp3init,
	.open = mp3open,
	.bufsz = mp3bufsz,
	.getfmt = mp3getfmt,
	.read = mp3read,
	.close = mp3close,
	.exit = mp3exit

};
