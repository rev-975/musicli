#include "globals.h"
#include <ctype.h>
#include <string.h>

char *songs[MAX_SONGS];
char *filenames[MAX_SONGS];
int song_count = 0;

int selected_index = 0; 

double speed = 1.0;
double volume = 50.0;

bool search_active = false;
char search_query[256] = {0};
int search_len = 0;

bool shuffle_mode = false;
bool repeat_mode = false;

double current_time = 0.0;
double total_time = 0.0;

mpv_handle *mpv = NULL;

// custom case-insensitive substring search because i can't get strcasestr to work :/
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
