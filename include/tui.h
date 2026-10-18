#ifndef TUI_H
#define TUI_H

#include "playlist.h"

// ANSII

// clears the screen and draws the layout
void tui_draw_dashboard(const Playlist *pl, int current_index, int is_playing, float current_time, float total_time);

// Updates only the progress bar line in-place without clearing the screen
void tui_update_status(float current_time, float total_time);

#endif