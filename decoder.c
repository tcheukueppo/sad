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
} decodermap[] = {
	{ ".wav",  &wavdecoder    },
	{ ".flac", &wavdecoder    },
	{ ".mp3",  &mp3decoder    },
	{ ".ogg",  &vorbisdecoder },
};

Decoder *
matchdecoder(const char *name)
{
	char *ext;
	int   i;

	ext = strrchr(name, '.');
	if (!ext)
		return NULL;
	for (i = 0; i < LEN(decodermap); i++)
		if (!strcasecmp(decodermap[i].ext, ext))
			return decodermap[i].decoder;
	return NULL;
}
