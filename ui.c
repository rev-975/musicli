#include <ncurses.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "playlist.h"
#include "controls.h"
#include "globals.h"

#define VISUALIZER_HEIGHT 6

double current_time = 0.0;
double total_time = 0.0;

static int scroll_offset = 0;
static double visualizer_phase = 0.0;

// --- helper functions ---
static void format_time(char *buf, size_t sz, double t) {
    if (t <= 0) { snprintf(buf, sz, "0:00"); return; }
    int sec = (int)t;
    int m = sec / 60;
    int s = sec % 60;
    snprintf(buf, sz, "%d:%02d", m, s);
}

static bool matches_filter(int idx) {
    if (!search_active || search_len == 0) return true;
    return strcasestr(filenames[idx], search_query) != NULL;
}

// --- visualizer ---
void draw_visualizer_region() {
    if (!visualizer_on) return;
    int base_row = LINES - VISUALIZER_HEIGHT;
    for (int row = 0; row < VISUALIZER_HEIGHT; row++) move(base_row + row, 0);
    int cols = COLS < 80 ? COLS : 80;
    visualizer_phase += 0.3;
    for (int x = 0; x < cols; x++) {
        int height = (int)((sin(x * 0.3 + visualizer_phase) + 1.0) * (VISUALIZER_HEIGHT / 2.0));
        for (int y = 0; y < height; y++) mvprintw(LINES - 1 - y, x, "|");
    }
}

// --- ui draw ---
void draw_ui() {
    clear();
    char curbuf[16], totbuf[16];
    format_time(curbuf, sizeof(curbuf), current_time);
    format_time(totbuf, sizeof(totbuf), total_time);

    // header
    mvprintw(0, 0, "mus player  [q] quit  [enter] play  [p/space] pause  [+/-] speed  [[] ] volume  [v] visualizer");
    mvprintw(1, 0, "currently playing: %s", playing_index >= 0 ? filenames[playing_index] : "(none)");
    mvprintw(2, 0, "            [j/k] up/down  [gg] top  [G] bottom  [h/l] prev/next  [/ search]  [←/→] seek  [s] shuffle  [r] repeat");
    mvprintw(3, 0, "time: %s / %s  speed: %.2fx  volume: %.0f%%  shuffle: %s  repeat: %s  visualizer: %s",
             curbuf, totbuf, speed, volume,
             shuffle_mode ? "on" : "off", repeat_mode ? "on" : "off",
             visualizer_on ? "on" : "off");

    if (search_active) mvprintw(4, 0, "search: %s", search_query);

    int header_rows = search_active ? 5 : 4;
    int start_row = header_rows + 1;
    int reserved = visualizer_on ? VISUALIZER_HEIGHT + 1 : 0;
    int available_rows = LINES - start_row - reserved;
    if (available_rows < 0) available_rows = 0;

    // filtered list
    int match_indices[MAX_SONGS];
    int match_count = 0;
    for (int i = 0; i < song_count; i++) if (matches_filter(i)) match_indices[match_count++] = i;

    if (match_count == 0) mvprintw(start_row, 2, "(no matches)");
    else {
        if (cursor_index >= match_count) cursor_index = match_count - 1;
        if (cursor_index < 0) cursor_index = 0;
        if (cursor_index < scroll_offset) scroll_offset = cursor_index;
        if (cursor_index >= scroll_offset + available_rows) scroll_offset = cursor_index - available_rows + 1;
        if (scroll_offset < 0) scroll_offset = 0;
        if (scroll_offset > match_count - available_rows) scroll_offset = match_count - available_rows;
        if (scroll_offset < 0) scroll_offset = 0;

        int printed = 0;
        for (int i = scroll_offset; i < match_count && printed < available_rows; i++, printed++) {
            int idx = match_indices[i];
            if (i == cursor_index) attron(A_REVERSE);
            if (idx == playing_index) attron(A_BOLD);
            mvprintw(start_row + printed, 2, "%s", filenames[idx]);
            if (idx == playing_index) attroff(A_BOLD);
            if (i == cursor_index) attroff(A_REVERSE);
        }
    }

    if (visualizer_on) {
        int spacer_row = LINES - VISUALIZER_HEIGHT - 1;
        move(spacer_row, 0); clrtoeol();
    }

    draw_visualizer_region();
    refresh();
}

// --- main loop ---
#include "controls.h"

void main_loop() {
    int ch;
    while (1) {
        ch = getch();
        if (ch == 'q') break;
        handle_keypress(ch);

        // process mpv events
        while (1) {
            mpv_event *ev = mpv_wait_event(mpv, 0);
            if (!ev || ev->event_id == MPV_EVENT_NONE) break;
            if (ev->event_id == MPV_EVENT_END_FILE) play_next_song();
        }

        mpv_get_property(mpv, "time-pos", MPV_FORMAT_DOUBLE, &current_time);
        mpv_get_property(mpv, "duration", MPV_FORMAT_DOUBLE, &total_time);

        draw_ui();
        usleep(10000);
    }
}

