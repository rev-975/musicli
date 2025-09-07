#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <stdbool.h>

#define MAX_SONGS 1000
#define MAX_PATH 4096

extern char *songs[MAX_SONGS];
extern char *filenames[MAX_SONGS];
extern int song_count;

extern int cursor_index;
extern int playing_index;

extern double speed;
extern double volume;

extern bool shuffle_mode;
extern bool repeat_mode;

void load_playlist(const char *playlist_path);
void free_playlist();
void play_song(int abs_index);
void play_next_song();
void play_prev_song();

#endif

