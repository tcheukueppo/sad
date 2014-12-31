#define RESAMPLEQUALITY SOXR_QQ

Outputdesc outputdescs[] = {
	{ "sndio", 16, 44100, 2, 0, 0, &sndiooutput, NULL },
	{ "alsa" , 16, 44100, 2, 1, 0, &alsaoutput,  NULL },
	{ "fifo",  16, 44100, 2, 0, 0, &fifooutput,  NULL },
};
