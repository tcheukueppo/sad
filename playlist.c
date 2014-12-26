#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sad.h"

static Playlist playlist;
static int      rollingid;

int
initplaylist(void)
{
	return 0;
}

Song *
addplaylist(const char *path)
{
	Song *s;
	Song **p;

	if (!playlist.nsongs || playlist.nsongs + 1 > playlist.maxsongs) {
		playlist.maxsongs += 4096;
		if (!(p = reallocarray(playlist.songs, playlist.maxsongs, sizeof(Song *))))
			err(1, "reallocarray");
		playlist.songs = p;
	}
	if (!(s = calloc(1, sizeof(Song))))
		err(1, "calloc");

	playlist.songs[playlist.nsongs] = s;
	strncpy(s->path, path, sizeof(s->path));
	s->path[sizeof(s->path) - 1] = '\0';
	s->id = rollingid++;
	s->state = 0;
	playlist.nsongs++;
	return s;
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
	playlist.nsongs = 0;
	rollingid = 0;
}
