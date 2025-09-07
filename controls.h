#ifndef CONTROLS_H
#define CONTROLS_H

void handle_keypress(int ch);
void toggle_pause();
void seek_relative(double sec);
void set_speed(double spd);
void set_volume(double vol);

#endif

