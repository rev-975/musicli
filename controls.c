#include "controls.h"
#include "globals.h"
#include "playlist.h"
#include <mpv/client.h>
#include <ncurses.h>
#include <ctype.h>

// toggle pause
void toggle_pause() {
    int paused = 0;
    mpv_get_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
    paused = !paused;
    mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
}

// seek relative
void seek_relative(double sec) {
    const char *args[] = {"seek", NULL, "relative", NULL};
    char buf[32];
    snprintf(buf, sizeof(buf), "%f", sec);
    args[1] = buf;
    mpv_command(mpv, args);
}

// set speed & volume
void set_speed(double spd) { mpv_set_property(mpv, "speed", MPV_FORMAT_DOUBLE, &spd); }
void set_volume(double vol) { mpv_set_property(mpv, "volume", MPV_FORMAT_DOUBLE, &vol); }

// handle key presses
void handle_keypress(int ch) {
    // build filtered/visible list for search mode
    int match_indices[MAX_SONGS];
    int match_count = 0;
    for (int i = 0; i < song_count; i++)
        if (!search_active || my_strcasestr(filenames[i], search_query))
            match_indices[match_count++] = i;

    // search mode
    if (search_active) {
        // clamp selected_index to filtered list bounds
        if (selected_index < 0) selected_index = 0;
        if (selected_index >= match_count) selected_index = match_count - 1;

        if (ch == 27) { // ESC
            search_active = false;
            search_len = 0;
            search_query[0] = '\0';
        }
        else if (ch == '\n') {
            if (match_count > 0) {
                int selected_song_index = match_indices[selected_index];
                play_song(selected_song_index);
            }
        }
        else if (ch == KEY_BACKSPACE || ch == 127) {
            if (search_len > 0) search_query[--search_len] = '\0';
            selected_index = 0; // reset to first match
        }
        else if (isprint(ch) && search_len < 255) {
            search_query[search_len++] = (char)ch;
            search_query[search_len] = '\0';
            selected_index = 0; // reset to first match
        }
        else if (ch == 'j') {
            selected_index++;
            if (selected_index >= match_count) selected_index = match_count - 1;
        }
        else if (ch == 'k') {
            selected_index--;
            if (selected_index < 0) selected_index = 0;
        }
        else if (ch == 'g') {
            selected_index = 0;
        }
        else if (ch == 'G') {
            selected_index = match_count - 1;
        }
        return;
    }

    if (selected_index < 0) selected_index = 0;
    if (selected_index >= song_count) selected_index = song_count - 1;

    switch (ch) {
        case 'j':
            selected_index++;
            if (selected_index >= song_count) selected_index = song_count - 1;
            break;
        case 'k':
            selected_index--;
            if (selected_index < 0) selected_index = 0;
            break;
        case 'g':
            selected_index = 0;
            break;
        case 'G':
            selected_index = song_count - 1;
            break;
        case 'h':
            play_prev_song();
            break;
        case 'l':
            play_next_song();
            break;
        case '\n':
            if (song_count > 0) {
                play_song(selected_index);
            }
            break;
        case 'p':
        case ' ':
            toggle_pause();
            break;
        case '+':
            speed += 0.1;
            set_speed(speed);
            break;
        case '-':
            speed = (speed > 0.1) ? speed - 0.1 : 0.1;
            set_speed(speed);
            break;
        case '[':
            if (volume > 0) volume -= 5;
            set_volume(volume);
            break;
        case ']':
            if (volume < 100) volume += 5;
            set_volume(volume);
            break;
        case KEY_LEFT:
            seek_relative(-5);
            break;
        case KEY_RIGHT:
            seek_relative(5);
            break;
        case '/':
            search_active = true;
            search_len = 0;
            search_query[0] = '\0';
            selected_index = 0; 
            break;
        case 's':
            shuffle_mode = !shuffle_mode;
            break;
        case 'r':
            repeat_mode = !repeat_mode;
            break;
    }
}
