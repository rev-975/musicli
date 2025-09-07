#include "ui.h"
#include "globals.h"
#include <ncurses.h>

void draw_ui() {
    clear();
    mvprintw(0, 0, "mus player  [q] quit  [enter] play  [p/space] pause  [+/-] speed  [[]/]] volume  [/]=search  [s] shuffle  [r] repeat");
    mvprintw(1, 0, "cursor: up/down=j/k  top/bottom=g/G  prev/next=h/l");
    mvprintw(2, 0, "speed: %.2fx  volume: %.0f  shuffle: %s  repeat: %s", 
             speed, volume, shuffle_mode ? "on" : "off", repeat_mode ? "on" : "off");

    if (search_active) mvprintw(3, 0, "search: %s", search_query);

    // display playlist
    int start_row = search_active ? 5 : 4;
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
