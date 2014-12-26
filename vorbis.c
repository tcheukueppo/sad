#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "sad.h"

static OggVorbis_File vf;
static int cursect;

static int
vorbisinit(void)
{
	return 0;
}

static int
vorbisopen(const char *name)
{
	int r;
	vorbis_info *vi;

	cursect = 0;
	r = ov_fopen(name, &vf);
	if (r < 0) {
		warnx("ov_fopen: failed");
		goto err0;
	}

	vi = ov_info(&vf, -1);
	if (!vi) {
		warnx("ov_info: failed");
		goto err0;
	}

	r = output->open(16, vi->rate, vi->channels);
	if (r < 0)
		goto err0;

	return 0;

err0:
	ov_clear(&vf);
	return -1;
}

static int
vorbisdecode(void *buf, int nbytes)
{
	int r;

	r = ov_read(&vf, buf, nbytes, 0, 2, 1, &cursect);
	if (r < 0) {
		warnx("ov_read: failed");
		return -1;
	} else if (r == 0)
		return r;
	return r;
}

static int
vorbisclose(void)
{
	int r;

	if (ov_clear(&vf) < 0)
		r = -1;
	if (output->close() < 0)
		r = -1;
	cursect = 0;
	return r;
}

static void
vorbisexit(void)
{
}

Decoder vorbisdecoder = {
	.init = vorbisinit,
	.open = vorbisopen,
	.decode = vorbisdecode,
	.close = vorbisclose,
	.exit = vorbisexit
};
