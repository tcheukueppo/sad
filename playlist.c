#include <sys/select.h>

#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "sad.h"

static Playlist playlist;
static int      rollingid;

int
initplaylist(void)
{
}

Song *
addplaylist(const char *path)
{
	Song  *s;

	s = &playlist.songs[playlist.nsongs];
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
		s = &playlist.songs[i];
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
		s = &playlist.songs[i];
		if (s->id == id)
			return s;
	}
	return NULL;
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
		s = &playlist.songs[i];
		dprintf(fd, "%d: %s\n", s->id, s->path);
	}
}

void
clearplaylist(void)
{
	playlist.nsongs = 0;
	rollingid = 0;
}
