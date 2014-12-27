#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "sad.h"

static struct {
	char   *name;
	int     bits;
	long    rate;
	int     channels;
	int     enabled;
	Output *output;
} outputs[] = {
	{
		.name = "sndio",
		.bits = 16,
		.rate = 44100,
		.channels = 2,
		.enabled = 1,
		.output = &sndiooutput
	},
};

int
openoutput(const char *name)
{
	int i;

	for (i = 0; i < LEN(outputs); i++) {
		if (!outputs[i].enabled)
			continue;
		if (strcmp(outputs[i].name, name))
			continue;
		return outputs[i].output->open(outputs[i].bits,
		                               outputs[i].rate,
		                               outputs[i].channels);
	}
	return -1;
}

int
closeoutput(const char *name)
{
	int i;

	for (i = 0; i < LEN(outputs); i++) {
		if (!outputs[i].enabled)
			continue;
		if (strcmp(outputs[i].name, name))
			continue;
		return outputs[i].output->close();
	}
}

int
closeoutputs(void)
{
	int i, r = 0;

	for (i = 0; i < LEN(outputs); i++)
		if (closeoutput(outputs[i].name) < 0)
			r = -1;
	return r;
}

int
openoutputs(void)
{
	int i, r = 0;

	for (i = 0; i < LEN(outputs); i++)
		if (openoutput(outputs[i].name) < 0)
			r = -1;
	return r;
}

int
playoutput(void *buf, size_t nbytes)
{
	int i, r = 0;

	for (i = 0; i < LEN(outputs); i++) {
		if (!outputs[i].enabled)
			continue;
		if (outputs[i].output->play(buf, nbytes) < 0)
			r = -1;
	}
	return r;
}

int
voloutput(int vol)
{
	int i, r = 0;

	for (i = 0; i < LEN(outputs); i++) {
		if (!outputs[i].enabled)
			continue;
		if (outputs[i].output->vol(vol) < 0)
			r = -1;
	}
	return r;
}
