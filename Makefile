CC = gcc
CFLAGS = -Wall -O2 `pkg-config --cflags mpv ncurses`
LDFLAGS = `pkg-config --libs mpv ncurses`

OBJS = main.o playlist.o ui.o controls.o

musplayer: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o musplayer

