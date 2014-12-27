#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "sad.h"

fd_set   master;
fd_set   rfds;
int      fdmax;
Output  *output = &sndiooutput;

static int
servlisten(const char *name)
{
	struct sockaddr_un sun;
	int    listenfd, r;
	socklen_t len;

	listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listenfd < 0)
		err(1, "socket");

	unlink(name);

	memset(&sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
	strlcpy(sun.sun_path, name, sizeof(sun.sun_path));

	len = sizeof(sun);
	r = bind(listenfd, (struct sockaddr *)&sun, len);
	if (r < 0)
		err(1, "bind");

	r = listen(listenfd, 5);
	if (r < 0)
		err(1, "listen");

	return listenfd;
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
		if (s->decoder->open(s->path) < 0) {
			s->state = NONE;
			return;
		}
		s->state = PLAYING;
		break;
	case PLAYING:
		if ((nbytes = s->decoder->decode(buf, sizeof(buf))) <= 0) {
			s->decoder->close();
			s->state = NONE;
			/* default to a repeat/cycle through mode */
			s = getnextsong();
			s->state = PREPARE;
			putcursong(s);
		} else {
			output->play(buf, nbytes);
		}
		break;
	}
}

int
main(void)
{
	struct timeval tv;
	int    listenfd, clifd, n, i;

	FD_ZERO(&master);
	FD_ZERO(&rfds);

	listenfd = servlisten("sock");
	FD_SET(listenfd, &master);
	fdmax = listenfd;

	initdecoders();

	while (1) {
		rfds = master;
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		n = select(fdmax + 1, &rfds, NULL, NULL, &tv);
		if (n < 0)
			err(1, "select");

		playaudio();

		for (i = 0; i <= fdmax; i++) {
			if (!FD_ISSET(i, &rfds))
				continue;
			if (i == listenfd) {
				clifd = servaccept(listenfd);
				FD_SET(clifd, &master);
				if (clifd > fdmax)
					fdmax = clifd;
				dprintf(clifd, "version %s\n", PROTOCOLVERSION);
			} else {
				if (docmd(i) < 0) {
					close(i);
					FD_CLR(i, &master);
				}
			}
		}
	}
	return 0;
}
