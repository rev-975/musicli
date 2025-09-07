# musplayer

**musplayer** is a terminal-based music player written in C, using `ncurses` for the interface and `mpv` as the backend for audio playback. It allows real-time speed and volume adjustments, playlist navigation, searching, and basic shuffle/repeat controls.

---

## Features

- Play songs from a playlist file (`.txt` with one song path per line)
- Navigate the playlist using a **vim-style interface**:
  - `j/k` → move cursor up/down
  - `g/G` → go to top/bottom
  - `h/l` → previous/next song
- Play/pause: `p` or spacebar
- Adjust speed: `+` / `-`
- Adjust volume: `[` / `]`
- Search songs: `/` (type search query, `ESC` to exit search)
- Shuffle mode: `s`
- Repeat mode: `r`
- Shows **currently playing song** with playback progress `[current/total]`
- Automatically goes to the next song when one ends

---

## Key Bindings

| Key       | Action                              |
|-----------|-------------------------------------|
| j / k     | Move cursor down / up               |
| g / G     | Go to top / bottom of playlist     |
| h / l     | Previous / next song                |
| enter     | Play selected song                  |
| p / space | Play / pause                        |
| + / -     | Increase / decrease speed           |
| [ / ]     | Decrease / increase volume          |
| /         | Start search                        |
| ESC       | Exit search mode                     |
| s         | Toggle shuffle mode                 |
| r         | Toggle repeat mode                  |
| q         | Quit player                          |

---

## Requirements

- `gcc` (tested on Linux)
- `mpv` library (`libmpv-dev`)
- `ncurses` library (`libncurses5-dev` or `libncurses-dev`)
- POSIX-compatible system

---

## Compilation

A `Makefile` is provided. To build the player:

```bash
make
```

This will compile the following files:

- `main.c`
- `playlist.c`
- `ui.c`
- `controls.c`
- `globals.c`

And link them into the executable `musplayer`.

Manual compilation:

```bash
gcc -Wall -O2 `pkg-config --cflags mpv ncurses` -c main.c
gcc -Wall -O2 `pkg-config --cflags mpv ncurses` -c playlist.c
gcc -Wall -O2 `pkg-config --cflags mpv ncurses` -c ui.c
gcc -Wall -O2 `pkg-config --cflags mpv ncurses` -c controls.c
gcc -Wall -O2 `pkg-config --cflags mpv ncurses` -c globals.c

gcc -o musplayer main.o playlist.o ui.o controls.o globals.o `pkg-config --libs mpv ncurses` -lm
```

---

## Usage

1. Create a playlist file with absolute paths to your songs:

```
/home/user/Music/song1.mp3
/home/user/Music/song2.mp3
/home/user/Music/song3.mp3
```

2. Run the player with the playlist:

```bash
./musplayer playlist.txt
```

3. Use the controls to navigate, play, pause, and adjust settings.

---

## Notes

- The player uses `mpv`’s API, so any format supported by mpv should work.
- Shuffle mode chooses songs randomly. Repeat mode will loop the current song indefinitely.
- The interface is entirely terminal-based and designed for keyboard navigation.
- Search is **case-insensitive** and filters the visible playlist.

---

