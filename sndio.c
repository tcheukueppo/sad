#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <sndio.h>
#include <stdio.h>

#include "sad.h"

static struct sio_hdl *hdl;
int sndiovolstatus = DEFAULTVOL;

static int sndiovol(int vol) {
  if (!sio_setvol(hdl, (SIO_MAXVOL * vol) / 100)) {
    warnx("sio_setvol: failed");
    return -1;
  }
  sndiovolstatus = vol;
  return 0;
}

static int sndioopen(Format *fmt) {
  struct sio_par par;

  hdl = sio_open(SIO_DEVANY, SIO_PLAY, 0);
  if (!hdl) {
    warnx("sio_open: failed");
    return -1;
  }

  sio_initpar(&par);
  par.bits = fmt->bits;
  par.rate = fmt->rate;
  par.pchan = fmt->channels;
  par.sig = 1;
  par.le = SIO_LE_NATIVE;

  if (!sio_setpar(hdl, &par) || !sio_getpar(hdl, &par)) {
    warnx("sio_{set,get}par: failed");
    goto err0;
  }

  if (par.bits != fmt->bits || par.rate != fmt->rate ||
      par.pchan != fmt->channels || par.le != SIO_LE_NATIVE || par.sig != 1) {
    warnx("unsupported audio params");
    goto err0;
  }

  if (!sio_start(hdl)) {
    warnx("sio_start: failed");
    goto err0;
  }

  return 0;

err0:
  sio_close(hdl);
  hdl = NULL;
  return -1;
}

static int sndioplay(void *buf, size_t nbytes) {
  return sio_write(hdl, buf, nbytes);
}

static int sndioclose(void) {
  if (hdl)
    sio_close(hdl);
  hdl = NULL;
  return 0;
}

Output sndiooutput = {
    .volstatus = &sndiovolstatus,
    .vol = sndiovol,
    .open = sndioopen,
    .play = sndioplay,
    .close = sndioclose,
};
