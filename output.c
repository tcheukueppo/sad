#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <soxr.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sad.h"

typedef struct {
  char *name;
  Format fmt;
  int enabled;
  int active;
  Output *output;
  soxr_t resampler;
} Outputdesc;

typedef struct {
  char *name;
  Format fmt;
  int enabled;
  Output *output;
} Outputcfg;

#include "config.h"

static Outputdesc outputdescs[LEN(outputcfgs)];

int initoutputs(void) {
  size_t i;

  for (i = 0; i < LEN(outputcfgs); i++) {
    outputdescs[i].name = outputcfgs[i].name;
    outputdescs[i].fmt = outputcfgs[i].fmt;
    outputdescs[i].enabled = outputcfgs[i].enabled;
    outputdescs[i].output = outputcfgs[i].output;
    outputdescs[i].active = 0;
    outputdescs[i].resampler = NULL;
  }
  return 0;
}

static int initresampler(Format *fmt, Outputdesc *desc) {
  soxr_quality_spec_t quality;
  soxr_io_spec_t iospec;

  quality = soxr_quality_spec(RESAMPLEQUALITY, 0);
  iospec = soxr_io_spec(SOXR_INT16_I, SOXR_INT16_I);

  if (desc->resampler)
    soxr_delete(desc->resampler);
  desc->resampler = soxr_create(fmt->rate, desc->fmt.rate, desc->fmt.channels,
                                NULL, &iospec, &quality, NULL);
  if (!desc->resampler) {
    warnx("soxr_create: failed to initialize resampler");
    return -1;
  }
  return 0;
}

int initresamplers(Format *fmt) {
  Outputdesc *desc;
  size_t i;
  int r = 0;

  for (i = 0; i < LEN(outputdescs); i++) {
    desc = &outputdescs[i];
    if (!desc->enabled)
      continue;
    if (initresampler(fmt, desc) < 0)
      r = -1;
  }
  return r;
}

static int openoutput(Outputdesc *desc) {
  if (desc->active)
    return 0;

  if (desc->output->open(&desc->fmt) < 0) {
    desc->active = 0;
    return -1;
  }

  printf("Opened %s output\n", desc->name);
  desc->active = 1;
  return 0;
}

int openoutputs(void) {
  Outputdesc *desc;
  size_t i;
  int r = 0;

  for (i = 0; i < LEN(outputdescs); i++) {
    desc = &outputdescs[i];
    if (!desc->enabled)
      continue;
    if (openoutput(desc) < 0)
      r = -1;
  }
  return r;
}

int getvol(void) {
  Outputdesc *desc;
  size_t i;
  for (i = 0; i < LEN(outputdescs); i++) {
    desc = &outputdescs[i];
    if (!desc->enabled)
      continue;
    return *desc->output->volstatus;
  }
  return -1;
}

static int closeoutput(Outputdesc *desc) {
  if (!desc->active)
    return 0;

  printf("Closed %s output\n", desc->name);
  desc->active = 0;
  return desc->output->close();
}

int closeoutputs(void) {
  Outputdesc *desc;
  size_t i;
  int r = 0;

  for (i = 0; i < LEN(outputdescs); i++) {
    desc = &outputdescs[i];
    if (!desc->enabled)
      continue;
    if (closeoutput(desc) < 0)
      r = -1;
  }
  return r;
}

static int playoutput(Format *fmt, Outputdesc *desc, void *buf, size_t nbytes) {
  soxr_error_t e;
  size_t inframes, outframes;
  size_t framesize;
  size_t idone, odone;
  void *inbuf;
  void *outbuf;
  float ratio;

  if (!desc->active)
    return 0;

  if (fmt->channels == 1 && desc->fmt.channels == 2) {
    /* perform mono to stereo conversion */
    inbuf = malloc(nbytes * 2);
    if (!inbuf)
      err(1, "malloc");
    s16monotostereo(buf, inbuf, nbytes / 2);
    nbytes *= 2;
  } else if (fmt->channels == 2 && desc->fmt.channels == 1) {
    /* perform stereo to mono conversion */
    inbuf = malloc(nbytes / 2);
    if (!inbuf)
      err(1, "malloc");
    s16stereotomono(buf, inbuf, nbytes / 4);
    nbytes /= 2;
  } else {
    inbuf = malloc(nbytes);
    if (!inbuf)
      err(1, "malloc");
    memcpy(inbuf, buf, nbytes);
  }

  if (fmt->rate == desc->fmt.rate) {
    if (desc->output->play(inbuf, nbytes) < 0) {
      free(inbuf);
      return -1;
    }
    free(inbuf);
    return 0;
  }

  /* perform SRC */
  framesize = (desc->fmt.bits + 7) / 8 * desc->fmt.channels;
  inframes = nbytes / framesize;
  ratio = (float)desc->fmt.rate / fmt->rate;
  outframes = inframes * ratio + 1;
  outbuf = malloc(outframes * framesize);
  if (!outbuf)
    err(1, "malloc");

  e = soxr_process(desc->resampler, inbuf, inframes, &idone, outbuf, outframes,
                   &odone);
  if (!e) {
    if (desc->output->play(outbuf, odone * framesize) < 0)
      return -1;
    free(inbuf);
    free(outbuf);
    return 0;
  }

  warnx("soxr_process: failed");
  free(inbuf);
  free(outbuf);
  return -1;
}

int playoutputs(Format *fmt, void *inbuf, size_t nbytes) {
  Outputdesc *desc;
  size_t i;
  int r = 0;

  for (i = 0; i < LEN(outputdescs); i++) {
    desc = &outputdescs[i];
    if (!desc->enabled)
      continue;
    if (playoutput(fmt, desc, inbuf, nbytes) < 0)
      r = -1;
  }
  return r;
}

int setvol(int vol) {
  Outputdesc *desc;
  size_t i;
  int r = 0;

  for (i = 0; i < LEN(outputdescs); i++) {
    desc = &outputdescs[i];
    if (!desc->enabled)
      continue;
    if (desc->output->vol(vol) < 0) {
      r = -1;
    }
    *desc->output->volstatus = vol;
  }
  return r;
}

int enableoutput(const char *name) {
  Outputdesc *desc;
  size_t i;

  for (i = 0; i < LEN(outputdescs); i++) {
    desc = &outputdescs[i];
    if (strcmp(desc->name, name))
      continue;
    if (openoutput(desc) < 0) {
      desc->enabled = 0;
      return -1;
    } else {
      desc->enabled = 1;
      return 0;
    }
  }
  return -1;
}

int disableoutput(const char *name) {
  Outputdesc *desc;
  size_t i;

  for (i = 0; i < LEN(outputdescs); i++) {
    desc = &outputdescs[i];
    if (strcmp(desc->name, name))
      continue;
    desc->enabled = 0;
    return closeoutput(desc);
  }
  return -1;
}
