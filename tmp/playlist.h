#ifndef PLAYLIST_H
#define PLAYLIST_H

void load_playlist(const char *playlist_path);
void play_song(int index);
void play_next_song();
void play_prev_song();

#endif
