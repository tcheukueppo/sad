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
mp3open(int fd)
{
	return mpg123_open_feed(handle) != MPG123_OK ? -1 : 0;
}

static int
mp3decode(int fd)
{
	unsigned char inbuf[8192];
	unsigned char outbuf[32768];
	ssize_t  n;
	size_t   sz;
	int      r;
	long     rate;
	int      channels, encoding, bits;

	n = read(fd, inbuf, sizeof(inbuf));
	if (n < 0)
		return -1;
	if (n == 0)
		return 0;

	mpg123_feed(handle, inbuf, n);
	r = mpg123_read(handle, outbuf, sizeof(outbuf), &sz);
	if (r == MPG123_NEW_FORMAT) {
		r = mpg123_getformat(handle, &rate, &channels,
		                     &encoding);
		if (r != MPG123_OK)
			return -1;
		bits = mpg123_encsize(encoding) * 8;
		mpg123_format_none(handle);
		mpg123_format(handle, rate, channels, encoding);
		curoutput->open(rate, channels, bits);
	}
	curoutput->play(outbuf, sz);
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
	.decode = mp3decode,
	.close = mp3close,
	.exit = mp3exit
};
