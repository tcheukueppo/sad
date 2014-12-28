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
	const char *errstr;

	if (!arg[0]) {
		dprintf(fd, "ERR expected volume\n");
		return;
	}

	vol = strtonum(arg, 0, 100, &errstr);
	if (errstr) {
		dprintf(fd, "ERR volume should be between [0, 100]\n");
		return;
	}
	if (setvol(vol) < 0) {
		dprintf(fd, "ERR failed to change volume\n");
		return;
	}
	dprintf(fd, "OK\n");
}

static void
cmdnext(int fd, char *arg)
{
	Song *s;

	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}

	s = getcursong();
	if (!s) {
		dprintf(fd, "ERR playlist is empty\n");
		return;
	}

	playsong(getnextsong());
	dprintf(fd, "OK\n");
}

static void
cmdpause(int fd, char *arg)
{
	Song *s;
	int   pause;
	const char *errstr;

	if (!arg[0]) {
		dprintf(fd, "ERR argument should be 0 or 1\n");
		return;
	}

	pause = strtonum(arg, 0, 1, &errstr);
	if (errstr) {
		dprintf(fd, "ERR argument should be 0 or 1\n");
		return;
	}

	s = getcursong();
	if (!s) {
		dprintf(fd, "ERR playlist is empty\n");
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
	case NONE:
		dprintf(fd, "ERR no song is active\n");
		return;
	}
	dprintf(fd, "OK\n");
}

static void
cmdplay(int fd, char *arg)
{
	Song *s, *cur;
	int   id;
	const char *errstr;

	cur = getcursong();
	if (!cur) {
		dprintf(fd, "ERR playlist is empty\n");
		return;
	}

	if (arg[0]) {
		id = strtonum(arg, 0, INT_MAX, &errstr);
		if (errstr) {
			dprintf(fd, "ERR invalid song id\n");
			return;
		}

		s = findsongid(id);
		if (!s) {
			dprintf(fd, "ERR cannot find song with given id\n");
			return;
		}
	} else {
		s = cur;
	}

	playsong(s);
	dprintf(fd, "OK\n");
}

static void
cmdprev(int fd, char *arg)
{
	Song *s;

	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}

	s = getcursong();
	if (!s) {
		dprintf(fd, "ERR playlist is empty\n");
		return;
	}

	playsong(getprevsong());
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
		dprintf(fd, "ERR playlist is empty\n");
		return;
	}

	stopsong(s);
	dprintf(fd, "OK\n");
}

static void
cmdadd(int fd, char *arg)
{
	const char *errstr;
	int id;

	if (!arg[0]) {
		dprintf(fd, "ERR expected ID\n");
		return;
	}

	id = strtonum(arg, 0, INT_MAX, &errstr);
	if (errstr) {
		dprintf(fd, "ERR invalid song id\n");
		return;
	}

	if (!addplaylist(id)) {
		dprintf(fd, "ERR cannot add song to playlist\n");
		return;
	}

	dprintf(fd, "OK\n");
}

static void
cmdclear(int fd, char *arg)
{
	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}

	stopsong(getcursong());
	clearplaylist();
	dprintf(fd, "OK\n");
}

static void
cmdempty(int fd, char *arg)
{
	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}

	stopsong(getcursong());
	clearplaylist();
	emptylibrary();
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
cmdlibrary(int fd, char *arg)
{
	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}
	dumplibrary(fd);
	dprintf(fd, "OK\n");
}

static void
cmdlearn(int fd, char *arg)
{
	if (!arg[0]) {
		dprintf(fd, "ERR expected argument file path\n");
		return;
	}
	if (!addlibrary(arg)) {
		dprintf(fd, "ERR failed to add song to library\n");
		return;
	}
	dprintf(fd, "OK\n");
}

static void
cmdclose(int fd, char *arg)
{	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}

	dprintf(fd, "OK\n");
	FD_CLR(fd, &master);
	close(fd);
}

static void
cmdkill(int fd, char *arg)
{
	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}
	dprintf(fd, "OK\n");
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
		dprintf(fd, "ERR expectede search string\n");
		return;
	}
	if (searchlibrary(fd, arg) != -1)
		dprintf(fd, "OK\n");
}

static void
cmdversion(int fd, char *arg)
{
	if (arg[0]) {
		dprintf(fd, "ERR unexpected argument\n");
		return;
	}
	dprintf(fd, "version 0.0\nOK\n");
}

static void
cmdenable(int fd, char *arg)
{
	if (!arg[0]) {
		dprintf(fd, "ERR expected output name\n");
		return;
	}
	if (enableoutput(arg) < 0) {
		dprintf(fd, "ERR failed to enable output %s\n", arg);
		return;
	}
	dprintf(fd, "OK\n");
}


static void
cmddisable(int fd, char *arg)
{
	if (!arg[0]) {
		dprintf(fd, "ERR expected output name\n");
		return;
	}
	if (disableoutput(arg) < 0) {
		dprintf(fd, "ERR failed to disable output %s\n", arg);
		return;
	}
	dprintf(fd, "OK\n");
}

static void
cmdwait(int fd, char *arg)
{
	if (!arg[0]) {
		dprintf(fd, "ERR expected event name\n");
		return;
	}

	if (addsubscribername(fd, arg) < 0) {
		dprintf(fd, "ERR unknown event type\n");
		return;
	}
	dprintf(fd, "OK\n");
}

static Cmd cmds[] = {
	{ "status",     cmdstatus     },
	{ "volume",     cmdvolume     },
	{ "next",       cmdnext       },
	{ "pause",      cmdpause      },
	{ "play",       cmdplay       },
	{ "prev",       cmdprev       },
	{ "stop",       cmdstop       },
	{ "add",        cmdadd        },
	{ "clear",      cmdclear      },
	{ "empty",      cmdempty      },
	{ "delete",     cmddelete     },
	{ "playlist",   cmdplaylist   },
	{ "library",    cmdlibrary    },
	{ "learn",      cmdlearn      },
	{ "close",      cmdclose      },
	{ "kill",       cmdkill       },
	{ "ping",       cmdping       },
	{ "search",     cmdsearch     },
	{ "version",    cmdversion    },
	{ "enable",     cmdenable     },
	{ "disable",    cmddisable    },
	{ "wait",       cmdwait       },
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
