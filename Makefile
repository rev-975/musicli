CC = gcc
CFLAGS = -Wall -O2 `pkg-config --cflags mpv ncurses`
LDFLAGS = `pkg-config --libs mpv ncurses` -lm

SOURCES = main.c playlist.c controls.c ui.c globals.c
OBJECTS = $(SOURCES:.c=.o)
TARGET = musplayer

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

