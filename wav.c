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
	sf = sf_open(name, SFM_READ, &sfinfo);
	if (!sf) {
		warnx("sf_open_fd: failed");
		return -1;
	}
	return output->open(16, sfinfo.samplerate, sfinfo.channels);
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
	int r;

	r = sf_close(sf);
	if (r != 0) {
		warnx("sf_close: failed");
		return -1;
	}
	return output->close();
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
