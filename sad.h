#define LEN(x) (sizeof (x) / sizeof *(x))
#define PROTOCOLVERSION "0.0"

typedef struct {
	char  *name;
	void (*fn)(int, int, char **);
} Cmd;

enum {
	NONE,
	PREPARE,
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
	int (*open)(int);
	int (*decode)(int);
	int (*close)(void);
	void (*exit)(void);
} Decoder;

typedef struct {
	int (*init)(void);
	int (*open)(int, int, int);
	void (*play)(void *, size_t);
	int (*close)(void);
	void (*exit)(void);
} Output;

/* sad.c */
extern fd_set   master;
extern fd_set   rfds;
extern int      fdmax;
extern Output  *output;
extern Decoder *decoder;

/* playlist.c */
Song *addplaylist(const char *);
Song *findsong(const char *);
Song *findsongid(int);
Song *getcursong(void);
void putcursong(Song *);

/* wav.c */
extern Decoder wavdecoder;

/* sndio.c */
extern Output sndiooutput;

/* tokenizer.c */
int gettokens(char *, char **, int, char *);
int tokenize(char *, char **, int);
