#include <mpv/client.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define MAX_SONGS 1000
#define MAX_PATH 4096

char *songs[MAX_SONGS];        
char *filenames[MAX_SONGS];    
int song_count = 0;
int selected_index = 0;
double speed = 1.0;
mpv_handle *mpv;


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

    char line[MAX_PATH];
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

void set_speed(double spd) {
    mpv_set_property(mpv, "speed", MPV_FORMAT_DOUBLE, &spd);
}

void draw_ui() {
    clear();
    mvprintw(0, 0, "mus player [q] quit  [Enter] play  [p/Space] pause  [+/-] speed");
    mvprintw(1, 0, "           [j/k] up/down  [gg/G] top/bottom  [h/l] prev/next ");
    mvprintw(2, 0, "Speed: %.2fx | Selected: %s", speed, filenames[selected_index]);

    for (int i = 0; i < song_count; i++) {
        if (i == selected_index) {
            attron(A_REVERSE);
        }
        mvprintw(i + 4, 2, "%s", filenames[i]);
        if (i == selected_index) {
            attroff(A_REVERSE);
        }
    }
    refresh();
}


void play_song(int index) {
    if (index < 0 || index >= song_count) return;

    const char *args[] = {"loadfile", songs[index], NULL};
    mpv_command(mpv, args);
    set_speed(speed); 
}

void toggle_pause() {
    int paused = 0;
    mpv_get_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
    paused = !paused;
    mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
}

void play_next_song() {
    if (selected_index < song_count - 1) {
        selected_index++;
        play_song(selected_index);
    }
}

void play_prev_song() {
    if (selected_index > 0) {
        selected_index--;
        play_song(selected_index);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <playlist.txt>\n", argv[0]);
        return 1;
    }

    load_playlist(argv[1]);

    if (song_count == 0) {
        fprintf(stderr, "No songs loaded.\n");
        return 1;
    }

    mpv = mpv_create();
    if (!mpv || mpv_initialize(mpv) < 0) {
        fprintf(stderr, "Failed to init mpv\n");
        return 1;
    }

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    timeout(100);  

    bool waiting_for_gg = false;
    int last_key = 0;

    draw_ui();

    int ch;
    while ((ch = getch()) != 'q') {
        switch (ch) {
            case 'j':
                if (selected_index < song_count - 1) selected_index++;
                draw_ui();
                waiting_for_gg = false;
                break;
            case 'k':
                if (selected_index > 0) selected_index--;
                draw_ui();
                waiting_for_gg = false;
                break;
            case 'g':
                if (last_key == 'g') {
                    selected_index = 0;
                    draw_ui();
                }
                waiting_for_gg = true;
                break;
            case 'G':
                selected_index = song_count - 1;
                draw_ui();
                waiting_for_gg = false;
                break;
            case 'h': // previous song
                play_prev_song();
                draw_ui();
                break;
            case 'l': // next song
                play_next_song();
                draw_ui();
                break;
            case '\n':
                play_song(selected_index);
                draw_ui();
                break;
            case 'p':
            case ' ':
                toggle_pause();
                break;
            case '+':
                speed += 0.1;
                set_speed(speed);
                draw_ui();
                break;
            case '-':
                speed = (speed > 0.1) ? speed - 0.1 : 0.1;
                set_speed(speed);
                draw_ui();
                break;
            default:
                waiting_for_gg = false;
                break;
        }

        last_key = ch;
        usleep(10000);
    }

    endwin();
    mpv_terminate_destroy(mpv);

    for (int i = 0; i < song_count; i++) {
        free(songs[i]);
        free(filenames[i]);
    }

    return 0;
}

