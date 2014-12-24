#include <sys/select.h>

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
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
		if (pause == 1) {
			s->state = PAUSED;
			FD_CLR(s->fd, &master);
		}
		break;
	case PAUSED:
		if (pause == 0) {
			s->state = PLAYING;
			FD_SET(s->fd, &master);
			if (s->fd > fdmax)
				fdmax = s->fd;
		}
		break;
	}
	printf("Song %s with id %d is %s\n",
	       s->path, s->id, s->state == PAUSED ? "paused" : "playing");
	dprintf(fd, "OK\n");
}

void
cmdplay(int fd, int argc, char **argv)
{
	Song *s;
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

	s->fd = open(s->path, O_RDONLY);
	if (s->fd < 0) {
		dprintf(fd, "ERR \"file doesn't exist\"\n");
		return;
	}
	FD_SET(s->fd, &master);
	if (s->fd > fdmax)
		fdmax = s->fd;
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
	curdecoder->close();
	curoutput->close();
	close(s->fd);
	s->fd = -1;
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

/* 0 on success, -1 on error or EOF */
int
docmd(int clifd)
{
	char    buf[BUFSIZ];
	int     i;
	ssize_t n;
	int     argc;
	char   *argv[2];

	n = read(clifd, buf, sizeof(buf) - 1);
	if (n <= 0)
		return -1;
	buf[n] = '\0';

	argc = tokenize(buf, argv, 2);
	for (i = 0; i < LEN(cmds); i++) {
		if (!strcmp(cmds[i].name, argv[0])) {
			cmds[i].fn(clifd, argc, argv);
			break;
		}
	}
	return 0;
}
