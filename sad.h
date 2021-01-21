#ifdef COMPAT
#include "compat.h"
#endif

#define LEN(x) (sizeof(x) / sizeof *(x))
#define PROTOCOLVERSION "0.0"
#define DEFAULTVOL 100

typedef struct {
  char *name;
  void (*fn)(int, char *);
} Cmd;

enum { NONE, PREPARE, PLAYING, PAUSED };

enum { REPEAT = 1 << 0, RANDOM = 1 << 1, SINGLE = 1 << 2 };

enum {
  EVSONGFINISHED,
};

typedef struct {
  unsigned int bits;
  unsigned int rate;
  unsigned int channels;
} Format;

typedef struct {
  int (*open)(Format *, const char *);
  int (*decode)(void *, int);
  int (*close)(void);
} Decoder;

typedef struct {
  int *volstatus;
  int (*vol)(int);
  int (*open)(Format *);
  int (*play)(void *, size_t);
  int (*close)(void);
} Output;

typedef struct {
  char path[PATH_MAX];
  int id;
  int state;
  Decoder *decoder;
  Format fmt;
} Song;

typedef struct {
  Song **songs;
  Song *cursong;
  size_t nsongs;
  size_t maxsongs;
  int mode;
} Playlist;

typedef struct {
  int event;
  char *name;
} Eventdesc;

/* sad.c */
extern fd_set master;
extern fd_set rfds;
extern int fdmax;

/* cmd.c */
int docmd(int);

/* playlist.c */
Song *addplaylist(const char *);
int rmplaylist(int);
Song *findsong(const char *);
Song *findsongid(int);
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
int getplaylistmode(void);
int searchplaylist(int, const char *);

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
int getvol(void);

/* notify.c */
extern Eventdesc Eventmap[];

int initnotifier(void);
int addsubscriber(int, int);
int addsubscribername(int, const char *);
int notify(int);
void removesubscriber(int);

/* pcm.c */
void s16monotostereo(short *, short *, size_t);
void s16stereotomono(short *, short *, size_t);
void s16tofloat(short *, float *, size_t);
void floattos16(float *, short *, size_t);
