#include <stdio.h>
#include "tui.h"
#include "audio.h" // Needed for audio_get_volume()

// Improved ANSI Escape code helpers to prevent trailing and flicker
#define CLEAR_SCREEN "\033[H\033[J" // Move cursor to top-left and clear downwards
#define HIDE_CURSOR  "\033[?25l"
#define SHOW_CURSOR  "\033[?25h"

#define COLOR_BOX    "\033[36m" // Cyan
#define COLOR_TEXT   "\033[37m" // White
#define COLOR_ACCENT "\033[32m" // Green
#define COLOR_RESET  "\033[0m"

void tui_draw_dashboard(const Playlist *pl, int current_index, int is_playing, float current_time, float total_time) {
    // Hide cursor and cleanly reset to top-left without messing up scroll history
    printf(HIDE_CURSOR CLEAR_SCREEN);

    printf(COLOR_BOX "==================================================\n");
    printf("                  C MUSIC PLAYER                  \n");
    printf("==================================================" COLOR_RESET "\n\n");

    printf(COLOR_TEXT "Status:   " COLOR_ACCENT "%s\n", is_playing ? "▶ PLAYING" : "⏸ PAUSED");
    
    if (pl->count > 0 && current_index >= 0 && current_index < pl->count) {
        printf(COLOR_TEXT "Now Playing: " COLOR_RESET "%s - %s\n", 
               pl->tracks[current_index].artist, 
               pl->tracks[current_index].title);
        printf(COLOR_TEXT "Track:       [%d / %d]\n" COLOR_RESET, current_index + 1, pl->count);

        int cur_min = (int)(current_time) / 60;
        int cur_sec = (int)(current_time) % 60;
        int tot_min = (int)(total_time) / 60;
        int tot_sec = (int)(total_time) % 60;

        int bar_width = 25;
        int progress = 0;
        if (total_time > 0) {
            progress = (int)((current_time / total_time) * bar_width);
            if (progress > bar_width) progress = bar_width;
        }

        printf(COLOR_TEXT "Time:        [%02d:%02d / %02d:%02d] ", cur_min, cur_sec, tot_min, tot_sec);
        printf(COLOR_ACCENT "[");
        for (int i = 0; i < bar_width; i++) {
            if (i < progress) printf("=");
            else if (i == progress) printf(">");
            else printf(" ");
        }
        printf("]" COLOR_RESET "\n");

        // Display current volume percentage
        float vol = audio_get_volume() * 100.0f;
        printf(COLOR_TEXT "Volume:      [%.0f%%]\n" COLOR_RESET, vol);

    } else {
        printf(COLOR_TEXT "Now Playing: " COLOR_RESET "None\n");
    }

    printf(COLOR_BOX "\n--------------------------------------------------\n");
    printf("Controls: [p]ause | [r]esume | [n]ext | [b]ack | [+|-] Vol | [q]uit\n");
    printf("--------------------------------------------------" COLOR_RESET "\n");
    
    // Restore cursor visibility and flush output
    printf(SHOW_CURSOR);
    fflush(stdout);
}

void tui_update_status(float current_time, float total_time) {
    int cur_min = (int)(current_time) / 60;
    int cur_sec = (int)(current_time) % 60;
    int tot_min = (int)(total_time) / 60;
    int tot_sec = (int)(total_time) % 60;

    int bar_width = 25;
    int progress = 0;
    if (total_time > 0) {
        progress = (int)((current_time / total_time) * bar_width);
        if (progress > bar_width) progress = bar_width;
    }

    // 1. Jump to Row 8 and update the Time progress bar (NO \n newline!)
    printf("\033[8;1H");
    printf(COLOR_TEXT "Time:        [%02d:%02d / %02d:%02d] ", cur_min, cur_sec, tot_min, tot_sec);
    printf(COLOR_ACCENT "[");
    for (int i = 0; i < bar_width; i++) {
        if (i < progress) printf("=");
        else if (i == progress) printf(">");
        else printf(" ");
    }
    printf("]" COLOR_RESET "          ");

    // 2. Jump to Row 9 and update the Volume line explicitly
    printf("\033[9;1H");
    float vol = audio_get_volume() * 100.0f;
    printf(COLOR_TEXT "Volume:      [%.0f%%]          " COLOR_RESET, vol);

    // 3. Return cursor down to the command line prompt area (Row 14)
    printf("\033[14;11H");
    fflush(stdout);
}