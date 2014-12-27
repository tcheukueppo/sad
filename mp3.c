#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <mpg123.h>

#include "sad.h"

static mpg123_handle *hdl;

static int
mp3init(void)
{
	if (mpg123_init() != MPG123_OK) {
		warnx("mpg123_init: failed");
		return -1;
	}

	hdl = mpg123_new(NULL, NULL);
	if (!hdl) {
		warnx("mpg123_new: failed");
		return -1;
	}
	return 0;
}

static int
mp3open(const char *name)
{
	int  r;
	long rate;
	int  channels, encoding, bits;

	r = mpg123_open(hdl, name);
	if (r != MPG123_OK) {
		warnx("mpg123_open: failed");
		return -1;
	}

	r = mpg123_getformat(hdl, &rate, &channels, &encoding);
	if (r != MPG123_OK) {
		warnx("mpg123_getformat: failed");
		goto err0;
	}

	setinputfmt(mpg123_encsize(encoding) * 8, rate, channels);
	return 0;
err0:
	mpg123_close(hdl);
	return -1;
}

static int
mp3decode(void *buf, int nbytes)
{
	size_t actual;
	int    r;

	r = mpg123_read(hdl, buf, nbytes, &actual);
	if (r == MPG123_DONE)
		return 0;
	else if (r != MPG123_OK) {
		warnx("mpg123_read: failed");
		return -1;
	}
	return actual;
}

static int
mp3close(void)
{
	if (mpg123_close(hdl) != MPG123_OK) {
		warnx("mpg123_close: failed");
		return -1;
	}
	return 0;
}

static void
mp3exit(void)
{
	if (hdl) {
		mpg123_delete(hdl);
		mpg123_exit();
	}
	hdl = NULL;
}

Decoder mp3decoder = {
	.init = mp3init,
	.open = mp3open,
	.decode = mp3decode,
	.close = mp3close,
	.exit = mp3exit
};
