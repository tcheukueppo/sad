#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <mpg123.h>

#include "sad.h"

static mpg123_handle *hdl;

static int
mp3open(Format *fmt, const char *name)
{
	int  r;
	long rate;
	int  channels, encoding;

	if (mpg123_init() != MPG123_OK) {
		warnx("mpg123_init: failed");
		return -1;
	}

	hdl = mpg123_new(NULL, NULL);
	if (!hdl) {
		warnx("mpg123_new: failed");
		goto err0;
	}

	r = mpg123_open(hdl, name);
	if (r != MPG123_OK) {
		warnx("mpg123_open: failed");
		goto err1;
	}

	r = mpg123_getformat(hdl, &rate, &channels, &encoding);
	if (r != MPG123_OK) {
		warnx("mpg123_getformat: failed");
		goto err2;
	}

	fmt->bits = mpg123_encsize(encoding) * 8;
	fmt->rate = rate;
	fmt->channels = channels;

	if (initresamplers(fmt) < 0)
		goto err2;

	return 0;

err2:
	mpg123_close(hdl);
err1:
	mpg123_delete(hdl);
err0:
	mpg123_exit();
	hdl = NULL;
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
	int r = 0;

	if (hdl) {
		if (mpg123_close(hdl) != MPG123_OK) {
			warnx("mpg123_close: failed");
			r = -1;
		}
		mpg123_delete(hdl);
		mpg123_exit();
	}
	hdl = NULL;
	return r;
}

Decoder mp3decoder = {
	.open = mp3open,
	.decode = mp3decode,
	.close = mp3close
};
