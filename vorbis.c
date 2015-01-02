#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <vorbis/vorbisfile.h>

#include "sad.h"

static OggVorbis_File vf;
static int cursect;

static int
vorbisopen(Format *fmt, const char *name)
{
	int r;
	vorbis_info *vi;

	r = ov_fopen(name, &vf);
	if (r < 0) {
		warnx("ov_fopen: failed");
		return -1;
	}

	vi = ov_info(&vf, -1);
	if (!vi) {
		warnx("ov_info: failed");
		goto err0;
	}

	fmt->bits = 16;
	fmt->rate = vi->rate;
	fmt->channels = vi->channels;

	if (initresamplers(fmt) < 0)
		goto err0;

	return 0;

err0:
	ov_clear(&vf);
	return -1;
}

static int
vorbisdecode(void *buf, int nbytes)
{
	int   r, decoded;
	char *p = buf;

	decoded = 0;
	while (nbytes > 0) {
		r = ov_read(&vf, &p[decoded], nbytes, 0, 2, 1, &cursect);
		if (r < 0) {
			warnx("ov_read: failed");
			return -1;
		} else if (r == 0) {
			break;
		}
		decoded += r;
		nbytes -= r;
	}
	return decoded;
}

static int
vorbisclose(void)
{
	return ov_clear(&vf);
}

Decoder vorbisdecoder = {
	.open = vorbisopen,
	.decode = vorbisdecode,
	.close = vorbisclose
};
