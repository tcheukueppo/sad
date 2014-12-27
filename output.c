#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "sad.h"

typedef struct {
	char   *name;
	int     bits;
	long    rate;
	int     channels;
	int     enabled;
	Output *output;
} Outputdesc;

#include "config.h"

static int  inbits;
static long inrate;
static int  inchannels;

int
openoutput(const char *name)
{
	Outputdesc *desc;
	int i;

	for (i = 0; i < LEN(Outputdescs); i++) {
		desc = &Outputdescs[i];
		if (!desc->enabled)
			continue;
		if (strcmp(desc->name, name))
			continue;
		return desc->output->open(desc->bits,
		                          desc->rate,
		                          desc->channels);
	}
	return -1;
}

int
openoutputs(void)
{
	int i, r = 0;

	for (i = 0; i < LEN(Outputdescs); i++) {
		if (openoutput(Outputdescs[i].name) < 0)
			r = -1;
		else
			printf("Opened %s output\n",
			       Outputdescs[i].name);
	}
	return r;
}

int
closeoutput(const char *name)
{
	Outputdesc *desc;
	int i;

	for (i = 0; i < LEN(Outputdescs); i++) {
		desc = &Outputdescs[i];
		if (!desc->enabled)
			continue;
		if (strcmp(desc->name, name))
			continue;
		return desc->output->close();
	}
	return -1;
}

int
closeoutputs(void)
{
	int i, r = 0;

	for (i = 0; i < LEN(Outputdescs); i++)
		if (closeoutput(Outputdescs[i].name) < 0)
			r = -1;
		else
			printf("Closed %s output\n",
			       Outputdescs[i].name);
	return r;
}

int
playoutput(void *buf, size_t nbytes)
{
	Outputdesc *desc;
	int i, r = 0;

	for (i = 0; i < LEN(Outputdescs); i++) {
		desc = &Outputdescs[i];
		if (!desc->enabled)
			continue;
		if (desc->output->play(buf, nbytes) < 0)
			r = -1;
	}
	return r;
}

int
setvol(int vol)
{
	Outputdesc *desc;
	int i, r = 0;

	for (i = 0; i < LEN(Outputdescs); i++) {
		desc = &Outputdescs[i];
		if (!desc->enabled)
			continue;
		if (desc->output->vol(vol) < 0)
			r = -1;
	}
	return r;
}

void
setinputfmt(int bits, long rate, int channels)
{
	inbits = bits;
	inrate = rate;
	inchannels = channels;
}
