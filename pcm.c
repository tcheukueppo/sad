#include <sys/select.h>

#include <limits.h>
#include <stdio.h>

#include "sad.h"

void s16monotostereo(short *in, short *out, size_t nsamples) {
  size_t i;

  for (i = 0; i < nsamples; i++) {
    out[i * 2] = in[i];
    out[i * 2 + 1] = in[i];
  }
}

void s16stereotomono(short *in, short *out, size_t nsamples) {
  size_t i;

  for (i = 0; i < nsamples; i++)
    out[i] = in[i * 2] / 2 + in[i * 2 + 1] / 2;
}

void s16tofloat(short *in, float *out, size_t nsamples) {
  size_t i;

  for (i = 0; i < nsamples; i++)
    out[i] = in[i] / 32768.f;
}

void floattos16(float *in, short *out, size_t nsamples) {
  size_t i;

  for (i = 0; i < nsamples; i++)
    out[i] = in[i] * 32768.f;
}
