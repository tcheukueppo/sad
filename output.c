#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <soxr.h>

#include "sad.h"

typedef struct {
	char   *name;
	int     bits;
	long    rate;
	int     channels;
	int     enabled;
	int     active;
	Output *output;
	soxr_t  resampler;
	int     inrate;
} Outputdesc;

#include "config.h"

static int
initresampler(Outputdesc *desc, int inrate)
{
	soxr_quality_spec_t quality;
	soxr_io_spec_t      iospec;
	int i;

	quality = soxr_quality_spec(RESAMPLEQUALITY, 0);
	iospec = soxr_io_spec(SOXR_INT16_I, SOXR_INT16_I);

	if (desc->resampler)
		soxr_delete(desc->resampler);
	desc->resampler = soxr_create(inrate, desc->rate,
	                              desc->channels,
	                              NULL,
	                              &iospec,
	                              &quality,
	                              NULL);
	if (!desc->resampler) {
		warnx("soxr_create: failed to initialize resampler");
		return -1;
	}

	desc->inrate = inrate;
	return 0;
}

int
initresamplers(int inrate)
{
	Outputdesc *desc;
	int i, r = 0;

	for (i = 0; i < LEN(outputdescs); i++) {
		desc = &outputdescs[i];
		if (!desc->enabled)
			continue;
		if (initresampler(desc, inrate) < 0)
			r = -1;
	}
	return r;
}

static int
openoutput(Outputdesc *desc)
{
	int i;

	if (desc->active)
		return 0;

	if (desc->output->open(desc->bits,
	                       desc->rate,
	                       desc->channels) < 0) {
		desc->active = 0;
		return -1;
	}

	printf("Opened %s output\n", desc->name);
	desc->active = 1;
	return 0;
}

int
openoutputs(void)
{
	Outputdesc *desc;
	int i, r = 0;

	for (i = 0; i < LEN(outputdescs); i++) {
		desc = &outputdescs[i];
		if (!desc->enabled)
			continue;
		if (openoutput(desc) < 0)
			r = -1;
	}
	return r;
}

static int
closeoutput(Outputdesc *desc)
{
	int i;

	if (!desc->active)
		return 0;

	if (desc->output->close() < 0) {
		desc->active = 1;
		return -1;
	}

	printf("Closed %s output\n", desc->name);
	desc->active = 0;
	return 0;
}

int
closeoutputs(void)
{
	Outputdesc *desc;
	int i, r = 0;

	for (i = 0; i < LEN(outputdescs); i++) {
		desc = &outputdescs[i];
		if (!desc->enabled)
			continue;
		if (closeoutput(desc) < 0)
			r = -1;
	}
	return r;
}

static int
playoutput(Outputdesc *desc, void *inbuf, size_t nbytes)
{
	soxr_error_t e;
	size_t inframes, outframes;
	size_t framesize;
	size_t idone, odone;
	void  *outbuf;
	float  ratio;
	int    i;

	if (!desc->active)
		return 0;

	if (desc->inrate == desc->rate) {
		if (desc->output->play(inbuf, nbytes) < 0)
			return -1;
		return 0;
	}

	/* perform SRC */
	framesize = (desc->bits + 7) / 8 * desc->channels;
	inframes = nbytes / framesize;
	ratio = (float)desc->rate / desc->inrate;
	outframes = inframes * ratio + 1;
	outbuf = malloc(outframes * framesize);
	if (!outbuf)
		err(1, "malloc");

	e = soxr_process(desc->resampler, inbuf, inframes,
	                 &idone, outbuf, outframes,
	                 &odone);
	if (!e) {
		if (desc->output->play(outbuf, odone * framesize) < 0)
			return -1;
		free(outbuf);
		return 0;
	}

	warnx("soxr_process: failed");
	free(outbuf);
	return -1;
}

int
playoutputs(void *inbuf, size_t nbytes)
{
	Outputdesc *desc;
	int i, r = 0;

	for (i = 0; i < LEN(outputdescs); i++) {
		desc = &outputdescs[i];
		if (!desc->active)
			continue;
		if (playoutput(desc, inbuf, nbytes) < 0)
			r = -1;
	}
	return r;
}

int
setvol(int vol)
{
	Outputdesc *desc;
	int i, r = 0;

	for (i = 0; i < LEN(outputdescs); i++) {
		desc = &outputdescs[i];
		if (!desc->active)
			continue;
		if (desc->output->vol(vol) < 0)
			r = -1;
	}
	return r;
}

int
enableoutput(const char *name)
{
	Outputdesc *desc;
	int i, r;

	for (i = 0; i < LEN(outputdescs); i++) {
		desc = &outputdescs[i];
		if (strcmp(desc->name, name))
			continue;
		if (desc->active)
			return -1;
		if (openoutput(desc) < 0) {
			desc->enabled = 0;
			return -1;
		} else {
			desc->enabled = 1;
			return 0;
		}
	}
	return -1;
}

int
disableoutput(const char *name)
{
	Outputdesc *desc;
	int i;

	for (i = 0; i < LEN(outputdescs); i++) {
		desc = &outputdescs[i];
		if (strcmp(desc->name, name))
			continue;
		if (!desc->active)
			return -1;
		if (closeoutput(desc) < 0) {
			desc->enabled = 1;
			return -1;
		} else {
			desc->enabled = 0;
			return 0;
		}
	}
	return -1;
}
