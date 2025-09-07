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
    bool auto_advance_enabled = true; // Control automatic song advancement
    
    while (1) {
        // handle key press
        ch = getch();
        if (ch == 'q') break;
        if (ch != ERR) {
            handle_keypress(ch);
            // Disable auto-advance when user interacts
            auto_advance_enabled = false;
        }

        // poll mpv events non-blocking
        mpv_event *event;
        while ((event = mpv_wait_event(mpv, 0)) && event->event_id != MPV_EVENT_NONE) {
            if (event->event_id == MPV_EVENT_END_FILE) {
                // Only auto-advance if enabled and not in search mode
                if (auto_advance_enabled && !search_active) {
                    if (repeat_mode) {
                        play_song(selected_index);  // repeat current
                    } else if (shuffle_mode) {
                        int next = rand() % song_count;
                        play_song(next);
                    } else {
                        play_next_song();
                    }
                }
            }
        }

        // Re-enable auto-advance after a short delay
        static int auto_advance_counter = 0;
        if (!auto_advance_enabled) {
            auto_advance_counter++;
            if (auto_advance_counter > 50) { // ~0.5 seconds
                auto_advance_enabled = true;
                auto_advance_counter = 0;
            }
        }

        // update song time for UI
        mpv_get_property(mpv, "time-pos", MPV_FORMAT_DOUBLE, &current_time);
        mpv_get_property(mpv, "duration", MPV_FORMAT_DOUBLE, &total_time);

        draw_ui();
        usleep(10000); // small delay
    }

    endwin();
    mpv_terminate_destroy(mpv);

    for (int i = 0; i < song_count; i++) {
        free(songs[i]);
        free(filenames[i]);
    }

    return 0;
}
