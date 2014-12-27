VERSION = 0.0

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

CFLAGS = -I/usr/local/include -g
LDFLAGS = -L /usr/local/lib
LDLIBS = -lsndfile -lmpg123 -lsndio -lvorbisfile

OBJ = sndio.o cmd.o mp3.o wav.o vorbis.o playlist.o sad.o decoder.o output.o fifo.o
BIN = sad

# non-OpenBSD
OBJ += compat/reallocarray.o
CFLAGS += -DCOMPAT

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

sndio.o: config.h sad.h
cmd.o: config.h sad.h
mp3.o: config.h sad.h
wav.o: config.h sad.h
vorbis.o: config.h sad.h
playlist.o: config.h sad.h
sad.o: config.h sad.h
decoder.o: config.h sad.h
output.o: config.h sad.h
fifo.o: config.h sad.h

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
