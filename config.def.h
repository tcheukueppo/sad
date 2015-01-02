#define RESAMPLEQUALITY SOXR_QQ

Outputcfg outputcfgs[] = {
	{
		.name = "sndio",
		.fmt = {
			.bits = 16,
			.rate = 48000,
			.channels = 2
		},
		.enabled = 0,
		.output = &sndiooutput
	},
	{
		.name = "alsa",
		.fmt = {
			.bits = 16,
			.rate = 48000,
			.channels = 2
		},
		.enabled = 1,
		.output = &alsaoutput
	},
	{
		.name = "fifo",
		.fmt = {
			.bits = 16,
			.rate = 48000,
			.channels = 2
		},
		.enabled = 0,
		.output = &fifooutput
	},
};
