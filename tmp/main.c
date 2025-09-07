#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <mpv/client.h>
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
    while ((ch = getch()) != 'q') {
        handle_keypress(ch);
        draw_ui();
    }

    endwin();
    mpv_terminate_destroy(mpv);

    for (int i = 0; i < song_count; i++) {
        free(songs[i]);
        free(filenames[i]);
    }

    return 0;
}
