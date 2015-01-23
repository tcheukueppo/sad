#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "arg.h"
#include "sad.h"

fd_set   master;
fd_set   rfds;
int      fdmax;
char    *argv0;

static int listenfd = -1;
static int isrunning = 1;
static char *socketpath = "/tmp/sad-sock";

static int
servlisten(const char *name)
{
	struct sockaddr_un sun;
	int    fd, r;
	socklen_t len;

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (fd < 0)
		err(1, "socket");

	unlink(name);

	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strlcpy(sun.sun_path, name, sizeof(sun.sun_path));

	len = sizeof(sun);
	r = bind(fd, (struct sockaddr *)&sun, len);
	if (r < 0)
		err(1, "bind");

	r = listen(fd, 5);
	if (r < 0)
		err(1, "listen");

	return fd;
}

static int
servaccept(int listenfd)
{
	struct sockaddr_un sun;
	int    clifd;
	socklen_t len;

	len = sizeof(sun);
	clifd = accept(listenfd, (struct sockaddr *)&sun, &len);
	if (clifd < 0)
		err(1, "accept");
	return clifd;
}

static void
playaudio(void)
{
	Song    *s;
	unsigned char buf[4096];
	int      nbytes;

	s = getcursong();
	if (!s)
		return;

	switch (s->state) {
	case PREPARE:
		if (s->decoder->open(&s->fmt, s->path) < 0) {
			s->state = NONE;
			return;
		}
		s->state = PLAYING;
		break;
	case PLAYING:
		if ((nbytes = s->decoder->decode(buf, sizeof(buf))) <= 0) {
			playsong(picknextsong());
			notify(EVSONGFINISHED);
		} else {
			playoutputs(&s->fmt, buf, nbytes);
		}
		break;
	}
}

static void
sighandler(int signum)
{
	if (signum == SIGINT || signum == SIGTERM)
		isrunning = 0;
}

static void
usage(void)
{
	fprintf(stderr, "usage: %s [-f sock]\n", argv0);
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct sigaction sa;
	struct timeval tv;
	int    clifd, n, i;

	ARGBEGIN {
	case 'f':
		socketpath = EARGF(usage());
		break;
	default:
		usage();
	} ARGEND;

	memset(&sa, 0, sizeof(sa));
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = sighandler;
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGINT, &sa, NULL);

	FD_ZERO(&master);
	FD_ZERO(&rfds);

	listenfd = servlisten(socketpath);
	FD_SET(listenfd, &master);
	fdmax = listenfd;

	initoutputs();
	openoutputs();
	initnotifier();

	playlistmode(REPEAT);

	while (isrunning) {
		rfds = master;
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		n = select(fdmax + 1, &rfds, NULL, NULL, &tv);
		if (n < 0) {
			if (errno == EINTR)
				goto fini;
			err(1, "select");
		}

		playaudio();

		for (i = 0; i <= fdmax; i++) {
			if (!FD_ISSET(i, &rfds))
				continue;
			if (i == listenfd) {
				clifd = servaccept(listenfd);
				FD_SET(clifd, &master);
				if (clifd > fdmax)
					fdmax = clifd;
			} else {
				if (docmd(i) < 0) {
					close(i);
					FD_CLR(i, &master);
					removesubscriber(i);
				}
			}
		}
	}
fini:
	for (i = 0; i <= fdmax; i++) {
		if (FD_ISSET(i, &master)) {
			close(i);
			FD_CLR(i, &master);
			removesubscriber(i);
		}
	}
	if (listenfd != -1)
		unlink(socketpath);
	return 0;
}
