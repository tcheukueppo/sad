#define RESAMPLEQUALITY SOXR_VHQ /* very high quality, higher cpu usage */

Outputdesc outputdescs[] = {
	{ "sndio", 16, 44100, 2, 0, 0, &sndiooutput, NULL, -1 },
	{ "alsa" , 16, 44100, 2, 1, 0, &alsaoutput,  NULL, -1 },
	{ "fifo",  16, 44100, 2, 0, 0, &fifooutput,  NULL, -1 },
};
