#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>

#include "sad.h"

static FILE *fp;
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

	fp = fopen(name, "r");
	if (!fp) {
		warn("fopen %s", name);
		return -1;
	}

	cursect = 0;
	r = ov_open_callbacks(fp, &vf, NULL, 0, OV_CALLBACKS_NOCLOSE);
	if (r < 0) {
		warnx("ov_open_callbacks: failed");
		return -1;
	}

	/* TODO: pull out params from file */
	return output->open(16, 44100, 2);
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

	if (fp) {
		fclose(fp);
		fp = NULL;
	}
	if (ov_clear(&vf) < 0)
		r = -1;
	if (output->close() < 0)
		r = -1;
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
