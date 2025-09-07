#include <mpv/client.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

#define MAX_SONGS 1000
#define MAX_PATH 4096
#define VISUALIZER_HEIGHT 6

char *songs[MAX_SONGS];
char *filenames[MAX_SONGS];
int song_count = 0;

int selected_index = 0;   // index within filtered list
int playing_index = -1;   // absolute index in songs[]

double speed = 1.0;
double volume = 70.0;

mpv_handle *mpv;

bool shuffle_mode = false;
bool repeat_mode = false;
bool visualizer_on = false;

bool search_active = false;
char search_query[256] = "";
int search_len = 0;

// time tracking
double current_time = 0.0;
double total_time = 0.0;

static int scroll_offset = 0;

char *basename_simple(const char *path) {
    const char *slash = strrchr(path, '/');
    return slash ? strdup(slash + 1) : strdup(path);
}

void load_playlist(const char *playlist_path) {
    FILE *f = fopen(playlist_path, "r");
    if (!f) {
        perror("failed to open playlist");
        exit(1);
    }
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

void set_speed(double spd) {
    mpv_set_property(mpv, "speed", MPV_FORMAT_DOUBLE, &spd);
}

void set_volume(double vol) {
    mpv_set_property(mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
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

void toggle_pause() {
    int paused = 0;
    mpv_get_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
    paused = !paused;
    mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
}

void seek_relative(double sec) {
    const char *args[] = {"seek", NULL, "relative", NULL};
    char buf[32];
    snprintf(buf, sizeof(buf), "%f", sec);
    args[1] = buf;
    mpv_command(mpv, args);
}

bool matches_filter(int idx) {
    if (!search_active || search_len == 0) return true;
    return strcasestr(filenames[idx], search_query) != NULL;
}

static void format_time(char *buf, size_t sz, double t) {
    if (t <= 0) {
        snprintf(buf, sz, "0:00");
        return;
    }
    int sec = (int)t;
    int m = sec / 60;
    int s = sec % 60;
    snprintf(buf, sz, "%d:%02d", m, s);
}

void draw_visualizer_region() {
    if (!visualizer_on) return;
    int base_row = LINES - VISUALIZER_HEIGHT;
    for (int row = 0; row < VISUALIZER_HEIGHT; row++) {
        move(base_row + row, 0);
        clrtoeol();
    }
    int cols = COLS < 80 ? COLS : 80;
    for (int x = 0; x < cols; x++) {
        int bar_height = rand() % VISUALIZER_HEIGHT;
        for (int y = 0; y < bar_height; y++) {
            mvprintw(LINES - 1 - y, x, "|");
        }
    }
}

void draw_ui() {
    clear();
    char curbuf[16], totbuf[16];
    format_time(curbuf, sizeof(curbuf), current_time);
    format_time(totbuf, sizeof(totbuf), total_time);

    mvprintw(0, 0, "mus player  [q] quit  [enter] play  [p/space] pause  [+/-] speed  [[] ] volume  [v] visualizer");
    mvprintw(1, 0, "            [j/k] up/down  [gg] top  [G] bottom  [h/l] prev/next  [/ search]  [←/→] seek");
    mvprintw(2, 0, "time: %s / %s  speed: %.2fx  volume: %.0f%%  shuffle: %s  repeat: %s  visualizer: %s",
             curbuf, totbuf, speed, volume,
             shuffle_mode ? "on" : "off", repeat_mode ? "on" : "off",
             visualizer_on ? "on" : "off");

    if (search_active) mvprintw(3, 0, "search: %s", search_query);

    int header_rows = search_active ? 4 : 3;
    int start_row = header_rows + 1;
    int reserved = visualizer_on ? VISUALIZER_HEIGHT + 1 : 0;
    int available_rows = LINES - start_row - reserved;
    if (available_rows < 0) available_rows = 0;

    // build filtered list
    int match_indices[MAX_SONGS];
    int match_count = 0;
    for (int i = 0; i < song_count; i++) {
        if (matches_filter(i)) match_indices[match_count++] = i;
    }

    if (match_count == 0) {
        mvprintw(start_row, 2, "(no matches)");
    } else {
        if (selected_index >= match_count) selected_index = match_count - 1;
        if (selected_index < 0) selected_index = 0;

        if (selected_index < scroll_offset) scroll_offset = selected_index;
        if (selected_index >= scroll_offset + available_rows)
            scroll_offset = selected_index - available_rows + 1;
        if (scroll_offset < 0) scroll_offset = 0;
        if (scroll_offset > match_count - available_rows)
            scroll_offset = match_count - available_rows;
        if (scroll_offset < 0) scroll_offset = 0;

        int printed = 0;
        for (int i = scroll_offset; i < match_count && printed < available_rows; i++, printed++) {
            int idx = match_indices[i];
            if (i == selected_index) attron(A_REVERSE);
            if (idx == playing_index) attron(A_BOLD);
            mvprintw(start_row + printed, 2, "%s", filenames[idx]);
            if (idx == playing_index) attroff(A_BOLD);
            if (i == selected_index) attroff(A_REVERSE);
        }
    }

    if (visualizer_on) {
        int spacer_row = LINES - VISUALIZER_HEIGHT - 1;
        move(spacer_row, 0);
        clrtoeol();
    }
    draw_visualizer_region();
    refresh();
}

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

    bool waiting_for_g = false;
    struct timespec g_time = {0};

    int ch;
    while (1) {
        ch = getch();
        if (ch == 'q') break;

        if (search_active) {
            if (ch != ERR) {
                if (ch == 27) { // esc
                    search_active = false;
                    search_len = 0;
                    search_query[0] = '\0';
                } else if (ch == '\n') {
                    // play selected song in filtered list
                    // handled below after building filtered list
                } else if (ch == KEY_BACKSPACE || ch == 127) {
                    if (search_len > 0) search_query[--search_len] = '\0';
                } else if (isprint(ch)) {
                    if (search_len < (int)sizeof(search_query) - 1) {
                        search_query[search_len++] = (char)ch;
                        search_query[search_len] = '\0';
                    }
                }
            }
        } else {
            if (ch != ERR) {
                switch (ch) {
                    case 'j': selected_index++; break;
                    case 'k': selected_index--; break;
                    case 'g':
                        if (waiting_for_g) {
                            selected_index = 0;
                            waiting_for_g = false;
                        } else {
                            waiting_for_g = true;
                            clock_gettime(CLOCK_MONOTONIC, &g_time);
                        }
                        break;
                    case 'G': selected_index = song_count - 1; break;
                    case 'h': play_prev_song(); break;
                    case 'l': play_next_song(); break;
                    case '\n': {
                        // build filtered list
                        int match_indices[MAX_SONGS], match_count = 0;
                        for (int i = 0; i < song_count; i++)
                            if (matches_filter(i)) match_indices[match_count++] = i;
                        if (match_count > 0 && selected_index < match_count) {
                            play_song(match_indices[selected_index]);
                        }
                        break;
                    }
                    case 'p':
                    case ' ': toggle_pause(); break;
                    case '+': speed += 0.1; set_speed(speed); break;
                    case '-': speed = (speed > 0.1) ? speed - 0.1 : 0.1; set_speed(speed); break;
                    case '[': if (volume > 0) volume -= 5; set_volume(volume); break;
                    case ']': if (volume < 100) volume += 5; set_volume(volume); break;
                    case KEY_LEFT: seek_relative(-5); break;
                    case KEY_RIGHT: seek_relative(5); break;
                    case 'v': visualizer_on = !visualizer_on; break;
                    case '/': search_active = true; search_len = 0; search_query[0] = '\0'; break;
                }
            }
        }

        if (waiting_for_g) {
            struct timespec now; clock_gettime(CLOCK_MONOTONIC, &now);
            long ms = (now.tv_sec - g_time.tv_sec) * 1000L + (now.tv_nsec - g_time.tv_nsec) / 1000000L;
            if (ms > 700) waiting_for_g = false;
        }

        while (1) {
            mpv_event *ev = mpv_wait_event(mpv, 0);
            if (!ev || ev->event_id == MPV_EVENT_NONE) break;
            if (ev->event_id == MPV_EVENT_END_FILE) play_next_song();
        }

        mpv_get_property(mpv, "time-pos", MPV_FORMAT_DOUBLE, &current_time);
        mpv_get_property(mpv, "duration", MPV_FORMAT_DOUBLE, &total_time);

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

