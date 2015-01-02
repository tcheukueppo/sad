VERSION = 0.0

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

CFLAGS = -I/usr/local/include
LDFLAGS = -L /usr/local/lib
LDLIBS = -lsndfile -lmpg123 -lsndio -lasound -lvorbisfile -lsoxr

OBJ =\
	alsa.o\
	cmd.o\
	decoder.o\
	fifo.o\
	library.o\
	mp3.o\
	notify.o\
	output.o\
	pcm.o\
	playlist.o\
	sad.o\
	sndio.o\
	vorbis.o\
	wav.o

BIN = sad

# non-OpenBSD
OBJ += compat/reallocarray.o
OBJ += compat/strlcat.o
OBJ += compat/strlcpy.o
OBJ += compat/strtonum.o
CFLAGS += -DCOMPAT

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

$(OBJ): arg.h compat.h config.h queue.h sad.h

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)

clean:
	rm -f $(BIN) $(OBJ)

.SUFFIXES: .def.h

.def.h.h:
	cp $< $@

.PHONY:
	all install uninstall clean
