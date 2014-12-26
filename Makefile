VERSION = 0.0

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

CFLAGS = -I/usr/local/include -g
LDFLAGS = -L /usr/local/lib
LDLIBS = -lsndfile -lmpg123 -lsndio -logg -lvorbis -lvorbisfile

OBJ = sndio.o cmd.o mp3.o wav.o vorbis.o playlist.o sad.o tokenizer.o decoder.o
BIN = sad

# non-OpenBSD
#OBJ += compat/reallocarray.o
#CFLAGS += -DCOMPAT

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

sndio.o: sad.h
cmd.o: sad.h
wav.o: sad.h
mp3.o: sad.h
vorbis.o: sad.h
playlist.o: sad.h
sad.o: sad.h
tokenizer.o: sad.h
decoder.o: sad.h

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)

clean:
	rm -f $(BIN) $(OBJ)
