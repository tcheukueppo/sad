#include <sys/select.h>
#include <sys/types.h>

#include <err.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sad.h"

static Library  library;
static int      rollingid;

Song *
addlibrary(const char *path)
{
	Decoder *d;
	Song    *s;

	d = matchdecoder(path);
	if (!d)
		return NULL;

	if (!library.nsongs || library.nsongs + 1 > library.maxsongs) {
		library.maxsongs += 4096;
		library.songs = reallocarray(library.songs, library.maxsongs, sizeof(Song *));
		if (!library.songs)
			err(1, "reallocarray");
	}

	s = malloc(sizeof(*s));
	if (!s)
		err(1, "malloc");

	library.songs[library.nsongs] = s;
	strlcpy(s->path, path, sizeof(s->path));
	s->id = rollingid++;
	s->state = NONE;
	s->decoder = d;
	library.nsongs++;
	return s;
}

Song *
findsongid(int id)
{
	Song *s;
	int   i;

	for (i = 0; i < library.nsongs; i++) {
		s = library.songs[i];
		if (s->id == id)
			return s;
	}
	return NULL;
}

void
dumplibrary(int fd)
{
	Song *s;
	int   i;

	for (i = 0; i < library.nsongs; i++) {
		s = library.songs[i];
		dprintf(fd, "%d: %s\n", s->id, s->path);
	}
}

static int
wregcomp(int fd, regex_t *preg, const char *regex, int cflags)
{
        char errbuf[BUFSIZ] = "";
        int r;

        if ((r = regcomp(preg, regex, cflags)) == 0)
                return r;

        regerror(r, preg, errbuf, sizeof(errbuf));
        dprintf(fd, "ERR invalid regex: %s\n", errbuf);
        return r;
}

int
searchlibrary(int fd, const char *search)
{
	Song *s;
	int   i;
	regex_t re;

	if (wregcomp(fd, &re, search, REG_EXTENDED | REG_ICASE))
		return -1; /* invalid regex */

	for (i = 0; i < library.nsongs; i++) {
		s = library.songs[i];
		if (!regexec(&re, s->path, 0, NULL, REG_NOSUB))
			dprintf(fd, "%d: %s\n", s->id, s->path);
	}
	regfree(&re);
	return 0;
}

void
emptylibrary(void)
{
	int i;

	for (i = 0; i < library.nsongs; i++) {
		free(library.songs[i]);
		library.songs[i] = NULL;
	}
	library.nsongs = 0;
	rollingid = 0;
}
