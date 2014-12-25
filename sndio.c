#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <sndio.h>

#include "sad.h"

static struct sio_hdl *hdl;

static int
sndioinit(void)
{
	return 0;
}

static int
sndiovol(int vol)
{
	if (hdl) {
		if (!sio_setvol(hdl, (SIO_MAXVOL * vol) / 100)) {
			warnx("sio_setvol: failed");
			return -1;
		}
	}
	return 0;
}

static int
sndioopen(int bits, int rate, int channels)
{
	struct sio_par par;

	hdl = sio_open(SIO_DEVANY, SIO_PLAY, 0);
	if (!hdl) {
		warnx("sio_open: failed");
		return -1;
	}

	sio_initpar(&par);
	par.bits = bits;
	par.rate = rate;
	par.pchan = channels;
	par.sig = 1;
	par.le = SIO_LE_NATIVE;

	if (!sio_setpar(hdl, &par) || !sio_getpar(hdl, &par)) {
		warnx("sio_{set,get}par: failed");
		goto err0;
	}

	if (par.bits != bits || par.rate != rate ||
	    par.pchan != channels || par.le != SIO_LE_NATIVE ||
	    par.sig != 1) {
		warnx("unsupported audio params");
		goto err0;
	}

	if (!sio_start(hdl)) {
		warnx("sio_start: failed");
		goto err0;
	}

	puts("Opened sndio output");
	return 0;

err0:
	sio_close(hdl);
	hdl = NULL;
	return -1;
}

static void
sndioplay(void *buf, size_t size)
{
	sio_write(hdl, buf, size);
}

static int
sndioclose(void)
{
	if (hdl)
		sio_close(hdl);
	hdl = NULL;
	puts("Closed sndio output");
}

static void
sndioexit(void)
{
}

Output sndiooutput = {
	.init = sndioinit,
	.vol = sndiovol,
	.open = sndioopen,
	.play = sndioplay,
	.close = sndioclose,
	.exit = sndioexit
};
