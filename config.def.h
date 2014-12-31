#define RESAMPLEQUALITY SOXR_QQ

Outputdesc outputdescs[] = {
	{ "sndio", 16, 44100, 2, 0, 0, &sndiooutput, NULL, -1 },
	{ "alsa" , 16, 44100, 2, 1, 0, &alsaoutput,  NULL, -1 },
	{ "fifo",  16, 44100, 2, 0, 0, &fifooutput,  NULL, -1 },
};
