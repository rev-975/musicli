#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>
#include <mpv/client.h>

// mpv handle
extern mpv_handle *mpv;

// search globals
extern bool search_active;
extern char search_query[256];
extern int search_len;

// visualizer toggle
extern bool visualizer_on;

#endif

