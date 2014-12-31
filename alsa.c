#include <sys/select.h>

#include <err.h>
#include <limits.h>
#include <stdio.h>
#include <alsa/asoundlib.h>

#include "sad.h"

static snd_pcm_t *hdl;
static int framesize;
static const char *device = "default"; /* TODO: make configurable? */

static int
alsavol(int vol)
{
	long min, max;
	snd_mixer_t *mixerhdl;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	const char *selem_name = "Master";

	if (snd_mixer_open(&mixerhdl, 0) < 0) {
		warnx("snd_mixer_open: failed");
		return -1;
	}

	if (snd_mixer_attach(mixerhdl, device) < 0) {
		warnx("snd_mixer_attach: failed");
		goto err0;
	}

	if (snd_mixer_selem_register(mixerhdl, NULL, NULL) < 0) {
		warnx("snd_mixer_selem_register: failed");
		goto err0;
	}

	if (snd_mixer_load(mixerhdl) < 0) {
		warnx("snd_mixer_load: failed");
		goto err0;
	}

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	elem = snd_mixer_find_selem(mixerhdl, sid);
	if (!elem) {
		warnx("snd_mixer_find_selem: failed");
		goto err0;
	}

	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	snd_mixer_selem_set_playback_volume_all(elem, vol * max / 100);

	snd_mixer_close(mixerhdl);
	return 0;
err0:
	snd_mixer_close(mixerhdl);
	return -1;
}

static int
alsaopen(Format *fmt)
{
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;
	int r;

	if (fmt->bits != 16) {
		warnx("unsupported number of bits");
		return -1;
	}

	if ((r = snd_pcm_open(&hdl, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		warnx("snd_pcm_open: %s\n", snd_strerror(r));
		return -1;
	}

	if ((r = snd_pcm_set_params(hdl, format, SND_PCM_ACCESS_RW_INTERLEAVED,
	     fmt->channels, fmt->rate, 1, 500000)) < 0) {
		warnx("send_pcm_set_params: %s\n", snd_strerror(r));
		goto err0;
	}

	framesize = (fmt->bits + 7) / 8 * fmt->channels;
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
