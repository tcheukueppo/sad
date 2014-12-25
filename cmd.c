#include <sys/select.h>

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sad.h"

void
cmdstatus(int fd, int argc, char **argv)
{
}

void
cmdvolume(int fd, int argc, char **argv)
{
}

void
cmdnext(int fd, int argc, char **argv)
{
}

void
cmdpause(int fd, int argc, char **argv)
{
	Song *s;
	int   i;
	int   pause;

	if (argc != 2) {
		dprintf(fd, "ERR \"usage: pause 0|1\"\n");
		return;
	}

	pause = atoi(argv[1]);
	if (pause != 0 && pause != 1) {
		dprintf(fd, "ERR \"usage: pause 0|1\"\n");
		return;
	}

	s = getcursong();
	if (!s) {
		dprintf(fd, "ERR \"no song is active\"\n");
		return;
	}

	switch (s->state) {
	case PLAYING:
		if (pause == 1)
			s->state = PAUSED;
		break;
	case PAUSED:
		if (pause == 0)
			s->state = PLAYING;
		break;
	}
	printf("Song %s with id %d is %s\n",
	       s->path, s->id, s->state == PAUSED ? "paused" : "playing");
	dprintf(fd, "OK\n");
}

void
cmdplay(int fd, int argc, char **argv)
{
	Song *s, *cur;
	int   id, i;

	if (argc != 2) {
		dprintf(fd, "ERR \"usage: play songid\"\n");
		return;
	}

	id = atoi(argv[1]);
	s = findsongid(id);
	if (!s) {
		dprintf(fd, "ERR \"invalid id\"\n");
		return;
	}

	cur = getcursong();
	if (cur) {
		decoder->close();
		cur->state = NONE;
	}

	s->state = PREPARE;
	putcursong(s);

	printf("Song %s with %d playing\n",
	       s->path, s->id);
}

void
cmdprev(int fd, int argc, char **argv)
{
}

void
cmdstop(int fd, int argc, char **argv)
{
	Song *s;

	if (argc != 1) {
		dprintf(fd, "ERR \"usage: stop\"\n");
		return;
	}

	s = getcursong();
	if (!s) {
		dprintf(fd, "ERR \"no song is active\"\n");
		return;
	}
	decoder->close();
	s->state = NONE;
	dprintf(fd, "OK\n");
}

void
cmdadd(int fd, int argc, char **argv)
{
	Song *s;

	if (argc != 2) {
		dprintf(fd, "ERR \"usage: add path\"\n");
		return;
	}
	if (access(argv[1], F_OK) < 0) {
		dprintf(fd, "ERR \"file doesn't exist\"\n");
		return;
	}
	s = addplaylist(argv[1]);
	printf("Added song with path %s and id %d\n",
	       s->path, s->id);
	dprintf(fd, "OK\n");
}

void
cmdclear(int fd, int argc, char **argv)
{
}

void
cmddelete(int fd, int argc, char **argv)
{
}

void
cmdplaylist(int fd, int argc, char **argv)
{
	if (argc != 1) {
		dprintf(fd, "ERR \"usage: playlist\"\n");
		return;
	}

	playlistdump(fd);
	dprintf(fd, "OK\n");
}

void
cmdclose(int fd, int argc, char **argv)
{
}

void
cmdkill(int fd, int argc, char **argv)
{
}

void
cmdping(int fd, int argc, char **argv)
{
	dprintf(fd, "OK\n");
}

Cmd cmds[] = {
	{ "status",   cmdstatus   },
	{ "volume",   cmdvolume   },
	{ "next",     cmdnext     },
	{ "pause",    cmdpause    },
	{ "play",     cmdplay     },
	{ "prev",     cmdprev     },
	{ "stop",     cmdstop     },
	{ "add",      cmdadd      },
	{ "clear",    cmdclear    },
	{ "delete",   cmddelete   },
	{ "playlist", cmdplaylist },
	{ "close",    cmdclose    },
	{ "kill",     cmdkill     },
	{ "ping",     cmdping     },
};

/* shamelessly taken from isakmpd ui.c */
int
docmd(int clifd)
{
	static  char *buf = 0;
	static  char *p;
	static  size_t sz;
	static  size_t resid;
	ssize_t n;
	char   *new_buf;
	int     i;
	int     argc;
	char   *argv[2];

	/* If no buffer, set it up.  */
	if (!buf) {
		sz = BUFSIZ;
		buf = malloc(sz);
		if (!buf)
			err(1, "malloc");
		p = buf;
		resid = sz;
	}
	/* If no place left in the buffer reallocate twice as large.  */
	if (!resid) {
		new_buf = reallocarray(buf, sz, 2);
		if (!new_buf)
			err(1, "reallocarray");
		buf = new_buf;
		p = buf + sz;
		resid = sz;
		sz *= 2;
	}
	n = read(clifd, p, resid);
	if (n <= 0)
		return -1;
	resid -= n;
	while (n--) {
		/*
		 * When we find a newline, cut off the line and feed it to the
		 * command processor.  Then move the rest up-front.
		 */
		if (*p == '\n') {
			*p = '\0';
			argc = tokenize(buf, argv, 2);
			for (i = 0; i < LEN(cmds); i++) {
				if (!strcmp(cmds[i].name, argv[0])) {
					cmds[i].fn(clifd, argc, argv);
					break;
				}
			}
			if (i == LEN(cmds))
				dprintf(clifd, "ERR \"unknown command\"\n");
			memmove(buf, p + 1, n);
			p = buf;
			resid = sz - n;
			continue;
		}
		p++;
	}
	return 0;
}
