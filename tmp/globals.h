#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>
#include <mpv/client.h>

#define MAX_SONGS 1000

extern char *songs[MAX_SONGS];
extern char *filenames[MAX_SONGS];
extern int song_count;

extern int cursor_index;
extern int current_playing_index;

extern double speed;
extern double volume;

extern bool search_active;
extern char search_query[256];
extern int search_len;

extern bool shuffle_mode;
extern bool repeat_mode;

extern mpv_handle *mpv;

// utility function for case-insensitive substring search
char *my_strcasestr(const char *haystack, const char *needle);

#endif

