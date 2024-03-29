#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <sndfile.h>
#include <stdio.h>

#include "sad.h"

static SNDFILE *sf;

static int wavopen(Format *fmt, const char *name) {
  SF_INFO sfinfo;
  int bits;

  sf = sf_open(name, SFM_READ, &sfinfo);
  if (!sf) {
    warnx("sf_open_fd: failed");
    return -1;
  }

  if (sfinfo.channels < 1 || sfinfo.channels > 2) {
    warnx("sfinfo: unsupported number of channels %d", sfinfo.channels);
    goto err0;
  }

  switch (sfinfo.format & 0xff) {
  case SF_FORMAT_PCM_S8:
    bits = 8;
    break;
  case SF_FORMAT_PCM_16:
    bits = 16;
    break;
  case SF_FORMAT_PCM_24:
    bits = 24;
    break;
  case SF_FORMAT_PCM_32:
    bits = 32;
    break;
  default:
    warnx("sfinfo: unsupported format");
    goto err0;
  }

  fmt->bits = bits;
  fmt->rate = sfinfo.samplerate;
  fmt->channels = sfinfo.channels;

  if (initresamplers(fmt) < 0)
    goto err0;

  return 0;
err0:
  sf_close(sf);
  sf = NULL;
  return -1;
}

static int wavdecode(void *buf, int nbytes) {
  return sf_read_short(sf, buf, nbytes / sizeof(short)) * sizeof(short);
}

static int wavclose(void) {
  int r = 0;

  if (sf) {
    r = sf_close(sf);
    if (r != 0) {
      warnx("sf_close: failed");
      r = -1;
    }
  }
  sf = NULL;
  return r;
}

Decoder wavdecoder = {.open = wavopen, .decode = wavdecode, .close = wavclose};
