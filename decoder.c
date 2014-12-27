#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "sad.h"

static struct {
	char    *ext;
	Decoder *decoder;
} Decodermap[] = {
	{ ".wav", &wavdecoder    },
	{ ".mp3", &mp3decoder    },
	{ ".ogg", &vorbisdecoder },
};

int
initdecoders(void)
{
	int i, r = 0;

	for (i = 0; i < LEN(Decodermap); i++)
		if (Decodermap[i].decoder->init() < 0)
			r = -1;
	return r;
}

Decoder *
matchdecoder(const char *name)
{
	char *ext;
	int   i;

	ext = strrchr(name, '.');
	if (!ext)
		return NULL;
	for (i = 0; i < LEN(Decodermap); i++)
		if (!strcasecmp(Decodermap[i].ext, ext))
			return Decodermap[i].decoder;
	return NULL;
}
