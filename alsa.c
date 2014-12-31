/* reference: http://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_min_8c-example.html */
#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <alsa/asoundlib.h>

#include "sad.h"

static snd_pcm_t *hdl;
static snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
static int framesize;
static const char *device = "default"; /* TODO: make configurable? */

static int
alsavol(int vol)
{
	return 0;
}

static int
alsaopen(int bits, int rate, int channels)
{
	int r;

	if ((r = snd_pcm_open(&hdl, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		warnx("snd_pcm_open: %s\n", snd_strerror(r));
		return -1;
	}
	if ((r = snd_pcm_set_params(hdl, format, SND_PCM_ACCESS_RW_INTERLEAVED,
	     channels, rate, 1, 500000)) < 0) {
		warnx("send_pcm_set_params: %s\n", snd_strerror(r));
		goto err0;
	}

	framesize = (bits + 7) / 8 * channels;
	return 0;

err0:
	snd_pcm_close(hdl);
	hdl = NULL;
	return -1;
}

static int
alsaplay(void *buf, size_t nbytes)
{
	snd_pcm_sframes_t frames;

	frames = snd_pcm_writei(hdl, buf, nbytes / framesize);
	if (frames < 0)
		frames = snd_pcm_recover(hdl, frames, 0);
	if (frames < 0) {
		warnx("snd_pcm_writei failed: %s\n", snd_strerror(frames));
		return -1;
	}
	return frames;
}

static int
alsaclose(void)
{
	if (hdl)
		snd_pcm_close(hdl);
	hdl = NULL;
	return 0;
}

Output alsaoutput = {
	.vol = alsavol,
	.open = alsaopen,
	.play = alsaplay,
	.close = alsaclose,
};
