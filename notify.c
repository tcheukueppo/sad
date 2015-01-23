#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "queue.h"
#include "sad.h"

Eventdesc Eventmap[] = {
	{ EVSONGFINISHED, "songfinished" },
};

struct subscriber {
	int clifd;
	int event;
	TAILQ_ENTRY(subscriber) entry;
};

static TAILQ_HEAD(subscribers, subscriber) subscribers;

int
initnotifier(void)
{
	TAILQ_INIT(&subscribers);
	return 0;
}

int
addsubscriber(int clifd, int event)
{
	struct subscriber *s;
	size_t i;

	for (i = 0; i < LEN(Eventmap); i++)
		if (Eventmap[i].event == event)
			break;
	if (i == LEN(Eventmap))
		return -1;

	TAILQ_FOREACH(s, &subscribers, entry)
		if (s->clifd == clifd &&
		    s->event == event)
			return 0; /* do not queue events */

	s = malloc(sizeof(*s));
	if (!s)
		err(1, "malloc");

	s->clifd = clifd;
	s->event = event;
	TAILQ_INSERT_TAIL(&subscribers, s, entry);

	return 0;
}

int
addsubscribername(int clifd, const char *name)
{
	size_t i;

	for (i = 0; i < LEN(Eventmap); i++)
		if (!strcmp(Eventmap[i].name, name))
			return addsubscriber(clifd, Eventmap[i].event);
	return -1;
}

int
notify(int event)
{
	struct subscriber *s, *tmp;
	size_t i;

	for (i = 0; i < LEN(Eventmap); i++)
		if (Eventmap[i].event == event)
			break;
	if (i == LEN(Eventmap))
		return -1;

	TAILQ_FOREACH_SAFE(s, &subscribers, entry, tmp) {
		if (s->event != event)
			continue;
		for (i = 0; i < LEN(Eventmap); i++) {
			if (Eventmap[i].event == s->event) {
				dprintf(s->clifd, "event: %s\n", Eventmap[i].name);
				dprintf(s->clifd, "OK\n");
				TAILQ_REMOVE(&subscribers, s, entry);
				free(s);
				break;
			}
		}
	}
	return 0;
}

void
removesubscriber(int clifd)
{
	struct subscriber *s, *tmp;

	TAILQ_FOREACH_SAFE(s, &subscribers, entry, tmp) {
		if (s->clifd == clifd) {
			TAILQ_REMOVE(&subscribers, s, entry);
			free(s);
		}
	}
}
