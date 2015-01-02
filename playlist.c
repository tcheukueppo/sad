#include <sys/select.h>
#include <sys/types.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "sad.h"

static Playlist playlist;
static int      rollingid;

Song *
addplaylist(const char *path)
{
	Song    *s;
	Decoder *d;

	if (access(path, F_OK) < 0)
		return NULL;

	d = matchdecoder(path);
	if (!d)
		return NULL;

	if (!playlist.nsongs || playlist.nsongs + 1 > playlist.maxsongs) {
		playlist.maxsongs += 4096;
		playlist.songs = reallocarray(playlist.songs, playlist.maxsongs, sizeof(Song *));
		if (!playlist.songs)
			err(1, "reallocarray");
	}

	s = malloc(sizeof(*s));
	if (!s)
		err(1, "malloc");

	playlist.songs[playlist.nsongs] = s;
	strlcpy(s->path, path, sizeof(s->path));
	s->id = rollingid++;
	s->state = NONE;
	s->decoder = d;
	if (!playlist.nsongs)
		playlist.cursong = s;
	playlist.nsongs++;
	return s;
}

int
rmplaylist(int id)
{
	Song *s;
	int   i;

	for (i = 0; i < playlist.nsongs; i++) {
		s = playlist.songs[i];
		if (s->id == id)
			break;
	}
	if (i == playlist.nsongs)
		return -1;

	free(s);
	s = getnextsong();
	memmove(&playlist.songs[i], &playlist.songs[i + 1],
	        (playlist.nsongs - i - 1) * sizeof(playlist.songs[i]));
	putcursong(s);
	playlist.nsongs--;
	return 0;
}

Song *
findsong(const char *path)
{
	Song *s;
	int   i;

	for (i = 0; i < playlist.nsongs; i++) {
		s = playlist.songs[i];
		if (!strcmp(s->path, path))
			return s;
	}
	return NULL;
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

void
clearplaylist(void)
{
	int i;

	for (i = 0; i < playlist.nsongs; i++) {
		free(playlist.songs[i]);
		playlist.songs[i] = NULL;
	}
	playlist.nsongs = 0;
	rollingid = 0;
	playlist.cursong = NULL;
}

Song *
picknextsong(void)
{
	Song *s;

	switch (playlist.mode) {
	case REPEAT:
		s = getnextsong();
		break;
	case RANDOM:
		srand(time(NULL));
		s = playlist.songs[rand() % playlist.nsongs];
		break;
	default:
		s = NULL;
		break;
	}
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

void
playlistmode(int mode)
{
	playlist.mode = mode;
}
