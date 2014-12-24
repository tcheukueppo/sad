#include <sys/select.h>
#include <sys/socket.h>
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
Output  *curoutput = &aooutput;
Decoder *curdecoder = &mp3decoder;

int
servlisten(const char *name)
{
	struct sockaddr_un sa;
	int    listenfd, r, len;

	listenfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (listenfd < 0)
		err(1, "socket");

	unlink(name);

	memset(&sa, 0, sizeof(sa));
	sa.sun_family = AF_UNIX;
	strcpy(sa.sun_path, name);

	len = strlen(sa.sun_path) + sizeof(sa.sun_family);
	r = bind(listenfd, (struct sockaddr *)&sa, len);
	if (r < 0)
		err(1, "bind");

	r = listen(listenfd, 5);
	if (r < 0)
		err(1, "listen");

	return listenfd;
}

int
servaccept(int listenfd)
{
	struct sockaddr_un sa;
	int    clifd, len;

	len = sizeof(sa);
	clifd = accept(listenfd, (struct sockaddr *)&sa, &len);
	if (clifd < 0)
		err(1, "accept");
	return clifd;
}

void
doaudio(void)
{
	Song    *s;
	unsigned char *buf;
	size_t   bufsz;
	long     rate;
	int      channels, bits;
	int      n;

	s = getcursong();
	if (!s)
		return;

	if (!FD_ISSET(s->fd, &rfds))
		return;

	switch (s->state) {
	case READYTOPLAY:
		curdecoder->open(s->path);
		curdecoder->getfmt(&rate, &channels, &bits);
		curoutput->putfmt(rate, channels, bits);
		curoutput->open();
		s->state = PLAYING;
		break;
	case PLAYING:
		bufsz = curdecoder->bufsz();
		buf = malloc(bufsz);
		if (!buf)
			err(1, "malloc");
		n = curdecoder->read(buf, bufsz);
		if (n != 0)
			curoutput->write(buf, n);
		free(buf);
	}
}

int
main(void)
{
	int listenfd, clifd, n, i;

	FD_ZERO(&master);
	FD_ZERO(&rfds);

	listenfd = servlisten("sock");
	FD_SET(listenfd, &master);
	fdmax = listenfd;

	curoutput->init();
	curdecoder->init();

	while (1) {
		rfds = master;
		n = select(fdmax + 1, &rfds, NULL, NULL, NULL);
		if (n < 0)
			err(1, "select");

		doaudio();

		for (i = 0; i <= fdmax; i++) {
			if (!FD_ISSET(i, &rfds))
				continue;
			if (i == listenfd) {
				clifd = servaccept(listenfd);
				FD_SET(clifd, &master);
				if (clifd > fdmax)
					fdmax = clifd;
				dprintf(clifd, "sad %s\n", PROTOCOLVERSION);
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
