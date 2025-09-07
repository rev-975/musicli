#include "ui.h"
#include "globals.h"
#include <ncurses.h>
#include <stdio.h>

// helper to format seconds into mm:ss
void format_time(double seconds, char *buf, size_t bufsize) {
    int mins = (int)(seconds / 60);
    int secs = (int)(seconds) % 60;
    snprintf(buf, bufsize, "%d:%02d", mins, secs);
}

void draw_ui() {
    clear();

    // display currently playing song with elapsed/total time
    if (current_playing_index >= 0 && current_playing_index < song_count) {
        char elapsed[16], total[16];
        format_time(current_time, elapsed, sizeof(elapsed));
        format_time(total_time, total, sizeof(total));
        mvprintw(0, 0, "currently playing: %s [%s/%s]", 
                 filenames[current_playing_index], elapsed, total);
    } else {
        mvprintw(0, 0, "currently playing: none");
    }

    // controls info
    mvprintw(1, 0, "mus player  [q] quit  [enter] play  [p/space] pause  [+/-] speed  [[]/]] volume  [/]=search  [s] shuffle  [r] repeat");
    mvprintw(2, 0, "cursor: up/down=j/k  top/bottom=g/G  prev/next=h/l");
    mvprintw(3, 0, "speed: %.2fx  volume: %.0f  shuffle: %s  repeat: %s", 
             speed, volume, shuffle_mode ? "on" : "off", repeat_mode ? "on" : "off");

    // search display
    if (search_active) mvprintw(4, 0, "search: %s", search_query);

    // display playlist
    int start_row = search_active ? 6 : 5;   // leave space for search if active
    int max_rows = LINES - start_row;
    int offset = 0;
    if (cursor_index >= max_rows) offset = cursor_index - max_rows + 1;

    for (int i = 0; i < max_rows && (i + offset) < song_count; i++) {
        int idx = i + offset;
        if (idx == cursor_index) attron(A_REVERSE);
        mvprintw(start_row + i, 2, "%s%s", filenames[idx], (idx == current_playing_index) ? " *" : "");
        if (idx == cursor_index) attroff(A_REVERSE);
    }

    refresh();
}

