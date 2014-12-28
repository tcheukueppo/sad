#include <sys/select.h>
#include <sys/types.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sad.h"

static Playlist playlist;
static int      rollingid;

Song *
addplaylist(int id)
{
	Song *s;

	s = findsongid(id);
	if (!s)
		return NULL;

	if (!playlist.nsongs || playlist.nsongs + 1 > playlist.maxsongs) {
		playlist.maxsongs += 4096;
		playlist.songs = reallocarray(playlist.songs, playlist.maxsongs, sizeof(Song *));
		if (!playlist.songs)
			err(1, "reallocarray");
	}

	playlist.songs[playlist.nsongs] = s;
	if (!playlist.nsongs)
		playlist.cursong = s;
	playlist.nsongs++;
	return s;
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
