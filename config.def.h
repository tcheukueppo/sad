#define RESAMPLEQUALITY SOXR_QQ
#define BITRATE 48000
#define BITDEPTH 16
#define CHANNELS 2
#define SOUNDSYS ALSA

/*
   Only need to edit below this line if you have multiple sound backends.
*/

#if SOUNDSYS == SNDIO
#define SNDIOON 1
#else
#define SNDIOON 0
#endif
#if SOUNDSYS == ALSA
#define ALSAON 1
#else
#define ALSAON 0
#endif
#if SOUNDSYS == FIFO
#define FIFOON 1
#else
#define FIFOON 0
#endif

Outputcfg outputcfgs[] = {
    {.name = "sndio",
     .fmt = {.bits = BITDEPTH, .rate = BITRATE, .channels = CHANNELS},
     .enabled = SNDIOON,
     .output = &sndiooutput},
    {.name = "alsa",
     .fmt = {.bits = BITDEPTH, .rate = BITRATE, .channels = CHANNELS},
     .enabled = ALSAON,
     .output = &alsaoutput},
    {.name = "fifo",
     .fmt = {.bits = BITDEPTH, .rate = BITRATE, .channels = CHANNELS},
     .enabled = FIFOON,
     .output = &fifooutput},
};
