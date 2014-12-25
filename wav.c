#include <sys/select.h>

#include <limits.h>
#include <stdio.h>
#include <sndfile.h>

#include "sad.h"

static SNDFILE *sf;
static SF_INFO sfinfo;

static int
wavinit(void)
{
	return 0;
}

static int
wavopen(const char *name)
{
	int bits;

	sf = sf_open(name, SFM_READ, &sfinfo);
	if (!sf) {
		warnx("sf_open_fd: failed");
		return -1;
	}

	if (sfinfo.channels < 1 || sfinfo.channels > 2) {
		warnx("sfinfo: unsupported number of channels %d",
		      sfinfo.channels);
		goto err0;
	}

	switch (sfinfo.format & 0xff) {
	case SF_FORMAT_PCM_S8:
		bits = 8;
		break;
	case SF_FORMAT_PCM_16:
		bits = 16;
		break;
	case SF_FORMAT_PCM_24:
		bits = 24;
		break;
	case SF_FORMAT_PCM_32:
		bits = 32;
		break;
	default:
		warnx("sfinfo: unsupported format");
		goto err0;
	}

	if (output->open(bits, sfinfo.samplerate, sfinfo.channels) < 0)
		goto err0;

	return 0;
err0:
	sf_close(sf);
	sf = NULL;
	return -1;
}

static int
wavdecode(void)
{
	sf_count_t n;
	short      buf[2048];

	n = sf_read_short(sf, buf, 2048);
	if (n > 0)
		output->play(buf, n * sizeof(short));
	return n * sizeof(short);
}

static int
wavclose(void)
{
	int r = 0;

	if (sf) {
		r = sf_close(sf);
		if (r != 0) {
			warnx("sf_close: failed");
			r = -1;
		}
	}
	sf = NULL;
	if (output->close() < 0)
		r = -1;
	return r;
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
