#define LEN(x) (sizeof (x) / sizeof *(x))
#define VERSION "0.0"

typedef struct {
	char  *name;
	void (*fn)(int, int, char **);
} Cmd;

enum {
	NONE,
	READYTOPLAY,
	PLAYING,
	PAUSED
};

typedef struct {
	char  path[PATH_MAX];
	int   id;
	int   fd;
	int   state;
} Song;

typedef struct {
	Song   songs[128];
	Song  *cursong;
	size_t nsongs;
} Playlist;

typedef struct {
	int (*init)(void);
	int (*open)(const char *);
	size_t (*bufsz)(void);
	int (*getfmt)(long *, int *, int *);
	int (*read)(void *, size_t);
	int (*close)(void);
	void (*exit)(void);
} Decoder;

typedef struct {
	int (*init)(void);
	int (*putfmt)(long, int, int);
	int (*open)(void);
	int (*write)(void *, size_t);
	int (*close)(void);
	void (*exit)(void);
} Output;

/* ao.c */
extern Output aooutput;

/* mp3.c */
extern Decoder mp3decoder;

/* playlist.c */
Song *addplaylist(const char *);
Song *findsong(const char *);
Song *findsongid(int);
Song *getcursong(void);
void putcursong(Song *);

/* sad.c */
extern fd_set   master;
extern fd_set   rfds;
extern int      fdmax;
extern Output  *curoutput;
extern Decoder *curdecoder;

/* tokenizer.c */
int gettokens(char *, char **, int, char *);
int tokenize(char *, char **, int);
