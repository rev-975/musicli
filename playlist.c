#include "playlist.h"
#include "globals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// helper to extract filename from path
char *basename(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? strdup(slash + 1) : strdup(path);
}

void load_playlist(const char *playlist_path) {
    FILE *f = fopen(playlist_path, "r");
    if (!f) {
        perror("failed to open playlist");
        exit(1);
    }

    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        if (song_count >= MAX_SONGS) break;
        line[strcspn(line, "\n")] = 0;
        if (strlen(line) == 0) continue;
        songs[song_count] = strdup(line);
        filenames[song_count] = basename(line);
        song_count++;
    }
    fclose(f);
}

void play_song(int index) {
    if (index < 0 || index >= song_count) return;
    const char *args[] = {"loadfile", songs[index], NULL};
    mpv_command(mpv, args);
    mpv_set_property(mpv, "speed", MPV_FORMAT_DOUBLE, &speed);
    mpv_set_property(mpv, "volume", MPV_FORMAT_DOUBLE, &volume);
    current_playing_index = index;
}

void play_next_song() {
    if (shuffle_mode) {
        int next = rand() % song_count;
        play_song(next);
        cursor_index = next;
    } else if (current_playing_index >= 0) {
        int next = current_playing_index + 1;
        if (next >= song_count) {
            if (repeat_mode) next = 0;
            else return;
        }
        play_song(next);
        cursor_index = next;
    }
}

void play_prev_song() {
    if (current_playing_index > 0) {
        int prev = current_playing_index - 1;
        play_song(prev);
        cursor_index = prev;
    }
}

