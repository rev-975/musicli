#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "playlist.h"
#include "controls.h"
#include "globals.h"
#include <mpv/client.h>

char *songs[MAX_SONGS];
char *filenames[MAX_SONGS];
int song_count = 0;

int cursor_index = 0;
int playing_index = -1;

double speed = 1.0;
double volume = 70.0;

bool shuffle_mode = false;
bool repeat_mode = false;

static char *basename_simple(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? strdup(slash + 1) : strdup(path);
}

void load_playlist(const char *playlist_path) {
    FILE *f = fopen(playlist_path, "r");
    if (!f) { perror("failed to open playlist"); exit(1); }
    char line[MAX_PATH];
    while (fgets(line, sizeof(line), f)) {
        if (song_count >= MAX_SONGS) break;
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;
        songs[song_count] = strdup(line);
        filenames[song_count] = basename_simple(line);
        song_count++;
    }
    fclose(f);
}

void free_playlist() {
    for (int i = 0; i < song_count; i++) {
        free(songs[i]);
        free(filenames[i]);
    }
}

void play_song(int abs_index) {
    if (abs_index < 0 || abs_index >= song_count) return;
    const char *args[] = {"loadfile", songs[abs_index], NULL};
    mpv_command(mpv, args);
    set_speed(speed);
    set_volume(volume);
    playing_index = abs_index;
}

void play_next_song() {
    if (repeat_mode && playing_index != -1) {
        play_song(playing_index);
    } else if (shuffle_mode) {
        int next = rand() % song_count;
        play_song(next);
    } else if (playing_index != -1) {
        int next = (playing_index + 1) % song_count;
        play_song(next);
    }
}

void play_prev_song() {
    if (playing_index == -1) return;
    int prev = (playing_index - 1 + song_count) % song_count;
    play_song(prev);
}


