#ifndef TUI_H
#define TUI_H

#include "playlist.h"

// ANSII
#define CLEAR_SCREEN "\033[2J"
#define MOVE_HOME    "\033[H"

// clears the screen and draws the layout
void tui_draw_dashbard(const Playlist *pl, int current_index, int is_playing);

// updating status of tracks without wiping entire box
void tui_update_status(const char *track_name, int is_playing);

#endif