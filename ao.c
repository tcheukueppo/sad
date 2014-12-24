#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>

#include <ao/ao.h>

#include "sad.h"

static ao_sample_format format;
static ao_device *dev;
static int driver;

static int
aoinit(void)
{
	ao_initialize();
	driver = ao_default_driver_id();
	return driver < 0 ? -1 : 0;
}

static int
aoputfmt(long rate, int channels, int bits)
{
	format.rate = rate;
	format.channels = channels;
	format.bits = bits;
	format.byte_format = AO_FMT_NATIVE;
	format.matrix = 0;
	return 0;
}

static int
aoopen(void)
{
	dev = ao_open_live(driver, &format, NULL);
	return !dev ? -1 : 0;
}

static int
aowrite(void *buf, size_t size)
{
	int r;

	r = ao_play(dev, buf, size);
	if (!r)
		return -1;
	return r;
}

static int
aoclose(void)
{
	return !ao_close(dev) ? -1 : 0;
}

static void
aoexit(void)
{
	ao_shutdown();
}

Output aooutput = {
	.init = aoinit,
	.open = aoopen,
	.putfmt = aoputfmt,
	.write = aowrite,
	.close = aoclose,
	.exit = aoexit
};
