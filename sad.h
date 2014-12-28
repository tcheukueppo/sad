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

typedef struct {
	int (*init)(void);
	int (*open)(const char *);
	int (*decode)(void *, int);
	int (*close)(void);
	void (*exit)(void);
} Decoder;

typedef struct {
	int (*vol)(int);
	int (*open)(int, int, int);
	int (*play)(void *, size_t);
	int (*close)(void);
} Output;

typedef struct {
	char     path[PATH_MAX];
	int      id;
	int      state;
	Decoder *decoder;
} Song;

typedef struct {
	Song **songs;
	Song  *cursong;
	size_t nsongs;
	size_t maxsongs;
} Playlist;

typedef struct {
	Song **songs;
	size_t nsongs;
	size_t maxsongs;
} Library;

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
Song *playnextsong(void);
Song *playprevsong(void);
void playsong(Song *);
void stopsong(Song *);

/* library.c */
Song *addlibrary(const char *);
Song *findsongid(int);
void dumplibrary(int);
int searchlibrary(int, const char *);
void clearlibrary(void);

/* wav.c */
extern Decoder wavdecoder;

/* mp3.c */
extern Decoder mp3decoder;

/* vorbis.c */
extern Decoder vorbisdecoder;

/* sndio.c */
extern Output sndiooutput;

/* fifo.c */
extern Output fifooutput;

/* tokenizer.c */
int gettokens(char *, char **, int, char *);
int tokenize(char *, char **, int);

/* decoder.c */
int initdecoders(void);
Decoder *matchdecoder(const char *);

/* output.c */
int openoutput(const char *);
int openoutputs(void);
int closeoutput(const char *);
int closeoutputs(void);
int playoutput(void *, size_t);
int setvol(int);
void setinputfmt(int, long, int);
