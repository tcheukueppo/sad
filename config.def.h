#define RESAMPLEQUALITY SOXR_QQ

Outputdesc outputdescs[] = {
	{
		.name = "sndio",
		.fmt = {
			.bits = 16,
			.rate = 44100,
			.channels = 2
		},
		.enabled = 0,
		.active = 0,
		.output = &sndiooutput,
		.resampler = NULL
	},
	{
		.name = "alsa",
		.fmt = {
			.bits = 16,
			.rate = 44100,
			.channels = 2
		},
		.enabled = 0,
		.active = 0,
		.output = &alsaoutput,
		.resampler = NULL
	},
	{
		.name = "fifo",
		.fmt = {
			.bits = 16,
			.rate = 44100,
			.channels = 2
		},
		.enabled = 0,
		.active = 0,
		.output = &fifooutput,
		.resampler = NULL
	},
};
