#include <sys/select.h>

#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sad.h"

static void
cmdstatus(int fd, char *arg)
{
}

static void
cmdvolume(int fd, char *arg)
{
	int vol;

	if (!arg[0]) {
		dprintf(fd, "ERR expected volume\n");
		return;
	}

	vol = atoi(arg);
	if (vol < 0 || vol > 100) {
		dprintf(fd, "ERR volume should be between [0, 100]\n");
		return;
	}
	if (output->vol(vol) < 0) {
		dprintf(fd, "ERR failed to change volume\n");
		return;
	}
	dprintf(fd, "OK\n");
}

static void
cmdnext(int fd, char *arg)
{
	Song *s, *next;

	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}

	s = getcursong();
	if (!s) {
		dprintf(fd, "ERR no song active\n");
		return;
	}

	s->decoder->close();
	s->state = NONE;
	next = getnextsong();
	next->state = PREPARE;
	putcursong(next);
	dprintf(fd, "OK\n");
}

static void
cmdpause(int fd, char *arg)
{
	Song *s;
	int   pause;

	if (!arg[0]) {
		dprintf(fd, "ERR argument should be 0 or 1\n");
		return;
	}

	pause = atoi(arg);
	if (pause != 0 && pause != 1) {
		dprintf(fd, "ERR argument should be 0 or 1\n");
		return;
	}

	s = getcursong();
	if (!s) {
		dprintf(fd, "ERR no song is active\n");
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

static void
cmdplay(int fd, char *arg)
{
	Song *s, *cur;
	int   id;

	if (!arg[0]) {
		dprintf(fd, "ERR expected song id\n");
		return;
	}

	id = atoi(arg);
	s = findsongid(id);
	if (!s) {
		dprintf(fd, "ERR invalid song id\n");
		return;
	}

	cur = getcursong();
	if (cur) {
		cur->decoder->close();
		cur->state = NONE;
	}

	s->state = PREPARE;
	putcursong(s);
	printf("Song %s with %d playing\n",
	       s->path, s->id);
}

static void
cmdprev(int fd, char *arg)
{
	Song *s, *prev;

	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}

	s = getcursong();
	if (!s) {
		dprintf(fd, "ERR no song active\n");
		return;
	}

	s->decoder->close();
	s->state = NONE;
	prev = getprevsong();
	prev->state = PREPARE;
	putcursong(prev);
	dprintf(fd, "OK\n");
}

static void
cmdstop(int fd, char *arg)
{
	Song *s;

	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}

	s = getcursong();
	if (!s) {
		dprintf(fd, "ERR no song is active\n");
		return;
	}
	s->decoder->close();
	s->state = NONE;
	dprintf(fd, "OK\n");
}

static void
cmdadd(int fd, char *arg)
{
	Song *s;

	if (!arg[0]) {
		dprintf(fd, "ERR expected file path\n");
		return;
	}
	if (access(arg, F_OK) < 0) {
		dprintf(fd, "ERR file doesn't exist: %s\n", arg);
		return;
	}
	s = addplaylist(arg);
	if (!s) {
		dprintf(fd, "ERR cannot add file: %s\n", arg);
		return;
	}
	printf("Added song with path %s and id %d\n",
	       s->path, s->id);
	dprintf(fd, "OK\n");
}

static void
cmdclear(int fd, char *arg)
{
	Song *s;

	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}

	s = getcursong();
	if (s) {
		s->decoder->close();
		s->state = NONE;
	}
	clearplaylist();
	dprintf(fd, "OK\n");
}

static void
cmddelete(int fd, char *arg)
{
}

static void
cmdplaylist(int fd, char *arg)
{
	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}
	dumpplaylist(fd);
	dprintf(fd, "OK\n");
}

static void
cmdclose(int fd, char *arg)
{
}

static void
cmdkill(int fd, char *arg)
{
	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}
	raise(SIGTERM);
}

static void
cmdping(int fd, char *arg)
{
	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}
	dprintf(fd, "pong\nOK\n");
}

static void
cmdsearch(int fd, char *arg)
{
	if (!arg[0]) {
		dprintf(fd, "ERR expects search string\n");
		return;
	}
	if (searchplaylist(fd, arg) != -1)
		dprintf(fd, "OK\n");
}

static Cmd cmds[] = {
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
	{ "search",   cmdsearch   }
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
	size_t cmdlen;
	char   *new_buf;
	int     i, c;
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
		new_buf = realloc(buf, sz * 2);
		if (!new_buf)
			err(1, "realloc");
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
			for (i = 0; i < LEN(cmds); i++) {
				cmdlen = strlen(cmds[i].name);
				if (!strncmp(cmds[i].name, buf, cmdlen) &&
				    (buf[cmdlen] == '\0' || isspace(buf[cmdlen]))) {
					/* strip leading whitespace */
					for (c = cmdlen; buf[c] && isspace(buf[c]); c++)
						;
					cmds[i].fn(clifd, &buf[c]);
					break;
				}
			}
			if (i == LEN(cmds))
				dprintf(clifd, "ERR unknown command\n");
			memmove(buf, p + 1, n);
			p = buf;
			resid = sz - n;
			continue;
		}
		p++;
	}
	return 0;
}
