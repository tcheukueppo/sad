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
	int     active;
	Output *output;
} Outputdesc;

#include "config.h"

int
openoutput(const char *name)
{
	Outputdesc *desc;
	int i;

	for (i = 0; i < LEN(Outputdescs); i++) {
		desc = &Outputdescs[i];
		if (strcmp(desc->name, name))
			continue;
		if (desc->active)
			return 0;
		if (desc->output->open(desc->bits,
		                       desc->rate,
		                       desc->channels) < 0) {
			desc->active = 0;
			return -1;
		} else {
			printf("Opened %s output\n", desc->name);
			desc->active = 1;
			return 0;
		}
	}
	return -1;
}

int
openoutputs(void)
{
	Outputdesc *desc;
	int i, r = 0;

	for (i = 0; i < LEN(Outputdescs); i++) {
		desc = &Outputdescs[i];
		if (!desc->enabled)
			continue;
		if (openoutput(desc->name) < 0)
			r = -1;
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
		if (strcmp(desc->name, name))
			continue;
		if (!desc->active)
			return 0;
		if (desc->output->close() < 0) {
			desc->active = 1;
			return -1;
		} else {
			printf("Closed %s output\n", desc->name);
			desc->active = 0;
			return 0;
		}
	}
	return -1;
}

int
closeoutputs(void)
{
	Outputdesc *desc;
	int i, r = 0;

	for (i = 0; i < LEN(Outputdescs); i++) {
		desc = &Outputdescs[i];
		if (!desc->enabled)
			continue;
		if (closeoutput(desc->name) < 0)
			r = -1;
	}
	return r;
}

int
playoutput(void *buf, size_t nbytes)
{
	Outputdesc *desc;
	int i, r = 0;

	for (i = 0; i < LEN(Outputdescs); i++) {
		desc = &Outputdescs[i];
		if (!desc->active)
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

	for (i = 0; i < LEN(Outputdescs); i++) {
		desc = &Outputdescs[i];
		if (strcmp(desc->name, name))
			continue;
		if (desc->active)
			return -1;
		if (openoutput(desc->name) < 0) {
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

	for (i = 0; i < LEN(Outputdescs); i++) {
		desc = &Outputdescs[i];
		if (strcmp(desc->name, name))
			continue;
		if (!desc->active)
			return -1;
		if (closeoutput(desc->name) < 0) {
			desc->enabled = 1;
			return -1;
		} else {
			desc->enabled = 0;
			return 0;
		}
	}
	return -1;
}
