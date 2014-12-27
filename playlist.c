#include <sys/select.h>
#include <sys/types.h>

#include <err.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sad.h"

static Playlist playlist;
static int      rollingid;

Song *
addplaylist(const char *path)
{
	Decoder *d;
	Song    *s;

	d = matchdecoder(path);
	if (!d)
		return NULL;

	if (!playlist.nsongs || playlist.nsongs + 1 > playlist.maxsongs) {
		playlist.maxsongs += 4096;
		if (!(playlist.songs = reallocarray(playlist.songs, playlist.maxsongs, sizeof(Song *))))
			err(1, "reallocarray");
	}
	if (!(s = calloc(1, sizeof(Song))))
		err(1, "calloc");

	playlist.songs[playlist.nsongs] = s;
	strncpy(s->path, path, sizeof(s->path));
	s->path[sizeof(s->path) - 1] = '\0';
	s->id = rollingid++;
	s->state = 0;
	s->decoder = d;
	if (!s->id)
		playlist.cursong = s;
	playlist.nsongs++;
	return s;
}

Song *
findsongid(int id)
{
	Song *s;
	int   i;

	for (i = 0; i < playlist.nsongs; i++) {
		s = playlist.songs[i];
		if (s->id == id)
			return s;
	}
	return NULL;
}

Song *
getnextsong(void)
{
	Song *s, *cur;
	int   i;

	cur = playlist.cursong;
	for (i = 0; i < playlist.nsongs; i++) {
		s = playlist.songs[i];
		if (s->id == cur->id)
			break;
	}
	if (i == playlist.nsongs)
		return NULL;
	if (i == playlist.nsongs - 1)
		s = playlist.songs[0];
	else
		s = playlist.songs[i + 1];
	return s;
}

Song *
getprevsong(void)
{
	Song *s, *cur;
	int   i;

	cur = playlist.cursong;
	for (i = 0; i < playlist.nsongs; i++) {
		s = playlist.songs[i];
		if (s->id == cur->id)
			break;
	}
	if (i == playlist.nsongs)
		return NULL;
	if (i == 0)
		s = playlist.songs[playlist.nsongs - 1];
	else
		s = playlist.songs[i - 1];
	return s;
}

Song *
getcursong(void)
{
	return playlist.cursong;
}

void
putcursong(Song *s)
{
	playlist.cursong = s;
}

void
dumpplaylist(int fd)
{
	Song *s;
	int   i;

	for (i = 0; i < playlist.nsongs; i++) {
		s = playlist.songs[i];
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
searchplaylist(int fd, const char *search)
{
	Song *s;
	int   i;
	regex_t re;

	if (wregcomp(fd, &re, search, REG_EXTENDED | REG_ICASE))
		return -1; /* invalid regex */

	for (i = 0; i < playlist.nsongs; i++) {
		s = playlist.songs[i];
		if (!regexec(&re, s->path, 0, NULL, REG_NOSUB))
			dprintf(fd, "%d: %s\n", s->id, s->path);
	}
	regfree(&re);
	return 0;
}

void
clearplaylist(void)
{
	playlist.nsongs = 0;
	rollingid = 0;
	playlist.cursong = NULL;
}

Song *
playnextsong(void)
{
	Song *s;

	stopsong(playlist.cursong);
	/* default to a repeat/cycle through mode */
	s = getnextsong();
	s->state = PREPARE;
	playlist.cursong = s;
	return s;
}

Song *
playprevsong(void)
{
	Song *s;

	stopsong(playlist.cursong);
	/* default to a repeat/cycle through mode */
	s = getprevsong();
	s->state = PREPARE;
	playlist.cursong = s;
	return s;
}

void
playsong(Song *new)
{
	stopsong(playlist.cursong);
	new->state = PREPARE;
	playlist.cursong = new;
}

void
stopsong(Song *s)
{
	if (s && s->state != NONE) {
		s->decoder->close();
		s->state = NONE;
	}
}
