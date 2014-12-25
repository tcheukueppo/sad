VERSION = 0.0

PREFIX = /usr/local
MANPREFIX = $(PREFIX)/man

CFLAGS = -I/usr/local/include -g
LDFLAGS = -L /usr/local/lib
LDLIBS = -lsndio

OBJ = sndio.o cmd.o wav.o playlist.o sad.o tokenizer.o
BIN = sad

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ) $(LDLIBS)

sndio.o: sad.h
cmd.o: sad.h
wav.o: sad.h
playlist.o: sad.h
sad.o: sad.h
tokenizer.o: sad.h

install: all
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -f $(BIN) $(DESTDIR)$(PREFIX)/bin

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/$(BIN)

clean:
	rm -f $(BIN) $(OBJ)
