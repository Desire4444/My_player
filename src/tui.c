#include <stdio.h>
#include "tui.h"

// ANSI Escape code helpers
#define CLEAR_SCREEN "\033[2J"
#define MOVE_HOME    "\033[H"
#define COLOR_BOX    "\033[36m" // Cyan
#define COLOR_TEXT   "\033[37m" // White
#define COLOR_ACCENT "\033[32m" // Green
#define COLOR_RESET  "\033[0m"

void tui_draw_dashbard(const Playlist *pl, int current_index, int is_playing){
    printf(CLEAR_SCREEN MOVE_HOME);

    printf(COLOR_BOX "==================================================\n");
    printf("                  C MUSIC PLAYER                  \n");
    printf("==================================================" COLOR_RESET "\n\n");

    printf(COLOR_TEXT "Status:   " COLOR_ACCENT "%s\n", is_playing ? "▶ PLAYING" : "⏸ PAUSED");

    if (pl->count > 0 && current_index >= 0 && current_index < pl->count) {
        printf(COLOR_TEXT "Now Playing: " COLOR_RESET "%s\n", pl->tracks[current_index].filepath );
        printf(COLOR_TEXT "Track    [%d / %d]\n" COLOR_RESET, current_index + 1, pl->count);
    } else {
        printf(COLOR_TEXT "Now Playing: " COLOR_RESET "None\n");
    }

    printf(COLOR_BOX "\n--------------------------------------------------\n");
    printf(" Controls: [p]ause | [r]esume | [n]ext | [b]ack | [q]uit\n");
    printf("--------------------------------------------------" COLOR_RESET "\n");
    printf("Command > ");
    fflush(stdout);
}