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
	int (*init)(void);
	int (*open)(const char *);
	int (*decode)(void *, int);
	int (*close)(void);
	void (*exit)(void);
} Decoder;

typedef struct {
	int (*vol)(int);
	int (*open)(int, int, int);
	void (*play)(void *, size_t);
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

/* sad.c */
extern fd_set   master;
extern fd_set   rfds;
extern int      fdmax;
extern Output  *output;

/* cmd.c */
int docmd(int);

/* playlist.c */
int initplaylist(void);
Song *addplaylist(const char *);
Song *findsong(const char *);
Song *findsongid(int);
Song *getnextsong(void);
Song *getprevsong(void);
Song *getcursong(void);
void putcursong(Song *);
void dumpplaylist(int);
void clearplaylist(void);

/* wav.c */
extern Decoder wavdecoder;

/* mp3.c */
extern Decoder mp3decoder;

/* sndio.c */
extern Output sndiooutput;

/* tokenizer.c */
int gettokens(char *, char **, int, char *);
int tokenize(char *, char **, int);

/* decoder.c */
int initdecoders(void);
Decoder *matchdecoder(const char *);
