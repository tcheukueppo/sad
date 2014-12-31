#ifdef COMPAT
#include "compat.h"
#endif

#define LEN(x) (sizeof (x) / sizeof *(x))
#define PROTOCOLVERSION "0.0"

typedef struct {
	char  *name;
	void (*fn)(int, char *);
} Cmd;

enum {
	NONE,
	PREPARE,
	PLAYING,
	PAUSED
};

enum {
	REPEAT,
	RANDOM
};

enum {
	EVSONGFINISHED,
};

typedef struct {
	int bits;
	int rate;
	int channels;
} Format;

typedef struct {
	int (*init)(void);
	int (*open)(Format *, const char *);
	int (*decode)(void *, int);
	int (*close)(void);
	void (*exit)(void);
} Decoder;

typedef struct {
	int (*vol)(int);
	int (*open)(Format *);
	int (*play)(void *, size_t);
	int (*close)(void);
} Output;

typedef struct {
	char     path[PATH_MAX];
	int      id;
	int      state;
	Decoder *decoder;
	Format   fmt;
} Song;

typedef struct {
	Song **songs;
	Song  *cursong;
	size_t nsongs;
	size_t maxsongs;
	int    mode;
} Playlist;

typedef struct {
	Song **songs;
	size_t nsongs;
	size_t maxsongs;
} Library;

typedef struct {
	int   event;
	char *name;
} Eventdesc;

/* sad.c */
extern fd_set   master;
extern fd_set   rfds;
extern int      fdmax;

/* cmd.c */
int docmd(int);

/* playlist.c */
Song *addplaylist(int);
Song *getnextsong(void);
Song *getprevsong(void);
Song *getcursong(void);
void putcursong(Song *);
void dumpplaylist(int);
void clearplaylist(void);
Song *picknextsong(void);
void playsong(Song *);
void stopsong(Song *);
void playlistmode(int);

/* library.c */
Song *addlibrary(const char *);
Song *findsong(const char *);
Song *findsongid(int);
void dumplibrary(int);
int searchlibrary(int, const char *);
void emptylibrary(void);

/* wav.c */
extern Decoder wavdecoder;

/* mp3.c */
extern Decoder mp3decoder;

/* vorbis.c */
extern Decoder vorbisdecoder;

/* sndio.c */
extern Output sndiooutput;

/* alsa.c */
extern Output alsaoutput;

/* fifo.c */
extern Output fifooutput;

/* tokenizer.c */
int gettokens(char *, char **, int, char *);
int tokenize(char *, char **, int);

/* decoder.c */
int initdecoders(void);
Decoder *matchdecoder(const char *);

/* output.c */
int initoutputs(void);
int initresamplers(Format *);
int openoutputs(void);
int closeoutputs(void);
int enableoutput(const char *);
int disableoutput(const char *);
int playoutputs(Format *, void *, size_t);
int setvol(int);

/* notify.c */
extern Eventdesc Eventmap[];

int initnotifier(void);
int addsubscriber(int, int);
int addsubscribername(int, const char *);
int notify(int);
void removesubscriber(int);
