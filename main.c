#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <mpv/client.h>
#include <unistd.h>
#include "globals.h"
#include "playlist.h"
#include "ui.h"
#include "controls.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: %s <playlist.txt>\n", argv[0]);
        return 1;
    }

    load_playlist(argv[1]);
    if (song_count == 0) {
        fprintf(stderr, "no songs loaded.\n");
        return 1;
    }

    mpv = mpv_create();
    if (!mpv || mpv_initialize(mpv) < 0) {
        fprintf(stderr, "failed to init mpv\n");
        return 1;
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(100);

    int ch;
    while (1) {
        // handle keypress
        ch = getch();
        if (ch == 'q') break;
        if (ch != ERR) handle_keypress(ch);

        // poll mpv events (non-blocking)
        mpv_event *event;
        while ((event = mpv_wait_event(mpv, 0)) && event->event_id != MPV_EVENT_NONE) {
            if (event->event_id == MPV_EVENT_END_FILE) {
                // song finished
                if (repeat_mode) {
                    play_song(current_playing_index);      // repeat current song
                } else if (shuffle_mode) {
                    int next = rand() % song_count;       // pick random song
                    play_song(next);
                } else {
                    play_next_song();                     // play next in playlist
                }
            }
        }

        // update current song time for UI
        if (current_playing_index >= 0) {
            mpv_get_property(mpv, "time-pos", MPV_FORMAT_DOUBLE, &current_time);
            mpv_get_property(mpv, "duration", MPV_FORMAT_DOUBLE, &total_time);
        }

        draw_ui();                     // redraw UI
        usleep(10000);                 // small delay
    }

    endwin();
    mpv_terminate_destroy(mpv);

    for (int i = 0; i < song_count; i++) {
        free(songs[i]);
        free(filenames[i]);
    }

    return 0;
}

