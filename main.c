#include <mpv/client.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include "playlist.h"
#include "ui.h"
#include "controls.h"
#include "globals.h"

// define globals
mpv_handle *mpv = NULL;
bool search_active = false;
char search_query[256] = "";
int search_len = 0;
bool visualizer_on = false;

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
    timeout(50);

    srand(time(NULL));

    main_loop();

    endwin();
    mpv_terminate_destroy(mpv);
    free_playlist();
    return 0;
}

