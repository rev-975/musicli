#define _GNU_SOURCE
#include "controls.h"
#include <mpv/client.h>
#include <ncurses.h>
#include <ctype.h>
#include "playlist.h"
#include "globals.h"

// portable version of strcasestr
char *my_strcasestr(const char *haystack, const char *needle) {
    if (!*needle) return (char*)haystack;
    for (; *haystack; haystack++) {
        const char *h = haystack;
        const char *n = needle;
        while (*h && *n && tolower((unsigned char)*h) == tolower((unsigned char)*n)) {
            h++; n++;
        }
        if (!*n) return (char*)haystack;
    }
    return NULL;
}


void toggle_pause() {
    int paused = 0;
    mpv_get_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
    paused = !paused;
    mpv_set_property(mpv, "pause", MPV_FORMAT_FLAG, &paused);
}

void seek_relative(double sec) {
    const char *args[] = {"seek", NULL, "relative", NULL};
    char buf[32]; snprintf(buf, sizeof(buf), "%f", sec);
    args[1] = buf;
    mpv_command(mpv, args);
}

void set_speed(double spd) { mpv_set_property(mpv, "speed", MPV_FORMAT_DOUBLE, &spd); }
void set_volume(double vol) { mpv_set_property(mpv, "volume", MPV_FORMAT_DOUBLE, &vol); }

void handle_keypress(int ch) {
    if (search_active) {
        if (ch == 27) { search_active = false; search_len = 0; search_query[0] = '\0'; }
        else if (ch == '\n') {
            int match_indices[MAX_SONGS], match_count = 0;
            for (int i = 0; i < song_count; i++) if (my_strcasestr(filenames[i], search_query)) match_indices[match_count++] = i;
            if (match_count > 0 && cursor_index < match_count) play_song(match_indices[cursor_index]);
        }
        else if (ch == KEY_BACKSPACE || ch == 127) { if (search_len > 0) search_query[--search_len] = '\0'; }
        else if (isprint(ch) && search_len < 255) { search_query[search_len++] = (char)ch; search_query[search_len] = '\0'; }
        return;
    }

    switch (ch) {
        case 'j': cursor_index++; break;
        case 'k': cursor_index--; break;
        case 'g': cursor_index = 0; break;
        case 'G': cursor_index = song_count - 1; break;
        case 'h': play_prev_song(); break;
        case 'l': play_next_song(); break;
        case '\n': {
            int match_indices[MAX_SONGS], match_count = 0;
            for (int i = 0; i < song_count; i++) if (!search_active || strcasestr(filenames[i], search_query)) match_indices[match_count++] = i;
            if (match_count > 0 && cursor_index < match_count) play_song(match_indices[cursor_index]);
            break;
        }
        case 'p': case ' ': toggle_pause(); break;
        case '+': speed += 0.1; set_speed(speed); break;
        case '-': speed = (speed > 0.1)? speed - 0.1 : 0.1; set_speed(speed); break;
        case '[': if (volume > 0) volume -= 5; set_volume(volume); break;
        case ']': if (volume < 100) volume += 5; set_volume(volume); break;
        case KEY_LEFT: seek_relative(-5); break;
        case KEY_RIGHT: seek_relative(5); break;
        case 'v': visualizer_on = !visualizer_on; break;
        case '/': search_active = true; search_len = 0; search_query[0] = '\0'; break;
        case 's': shuffle_mode = !shuffle_mode; break;
        case 'r': repeat_mode = !repeat_mode; break;
    }
}

