#include <poll.h>
#include <stdio.h>
#include <sys/poll.h>
#include <unistd.h>
#include "scanner.h"
#include "playlist.h"
#include "audio.h"
#include "tui.h"

int main(int argc, char *argv[]){
    printf("Initailizing local music player core......\n");

    if(argc < 2){
        fprintf(stderr, "Usage: %s <path_to_music_folder>\n", argv[0]);
        return 1;
    }
    // Initialize our playlist que with a starting capacity of 10
    Playlist my_playlist;
    playlist_init(&my_playlist, 10);

    printf("Scanning directory (Including subfolders): %s\n", argv[1]);
    scan_music_directory_recursive(argv[1], &my_playlist);

    printf("-----Playlist Loaded-----");
    printf("Number of tracks found: %d\n", my_playlist.count);

    if (my_playlist.count == 0) {
        printf("No music track found in this directory.\n");
        Playlist_free(&my_playlist);
        return 0;
    }

    for (int i = 0; i < my_playlist.count; i++) {
        printf("[%d] %s\n", i + 1, my_playlist.tracks[i].filename);
    }

    // Initialize audio engine
    if (!audio_engine_init()) {
        Playlist_free(&my_playlist);
        return 0;
    }

    // play first track as a test
    printf("\nAttempting to play track 1......\n");
    audio_play_track(my_playlist.tracks[0].filepath);

    
    // later an interactive loop will be created for controls
int current_index = 0;
    int is_playing = 1;
    audio_play_track(my_playlist.tracks[current_index].filepath);

    struct pollfd pfd;
    pfd.fd = 0; 
    pfd.events = POLLIN;

    int tick_counter = 0;
    
    // Draw initial full dashboard once
    tui_draw_dashboard(&my_playlist, current_index, is_playing, audio_get_cursor_seconds(), audio_get_length_seconds());
    printf("Command > ");
    fflush(stdout);
    
    while (1) {
        int ret = poll(&pfd, 1, 100); // 100ms tick

        if (ret > 0) {
            char command;
            scanf(" %c", &command);

            if (command == 'p') {
                audio_pause_track();
                is_playing = 0;
            } 
            else if (command == 'r') {
                audio_resume_track();
                is_playing = 1;
            } 
            else if (command == 'n') {
                current_index = (current_index + 1) % my_playlist.count;
                audio_play_track(my_playlist.tracks[current_index].filepath);
                is_playing = 1;
            } 
            else if (command == 'b') {
                current_index = (current_index - 1 + my_playlist.count) % my_playlist.count;
                audio_play_track(my_playlist.tracks[current_index].filepath);
                is_playing = 1;
            }
            else if (command == '+'){
                float current_volume = audio_get_volume();
                audio_set_volume(current_volume + 0.1f);
            }
            else if (command == '-'){
                float current_volume = audio_get_volume();
                audio_set_volume(current_volume - 0.1f);
            }
            else if (command == 'q') {
                printf("\033[H\033[J"); // Clean clear on exit
                printf("Quitting C Music Player...\n");
                break;
            }
            
            // Full redraw on state/track changes
            tui_draw_dashboard(&my_playlist, current_index, is_playing, audio_get_cursor_seconds(), audio_get_length_seconds());
            printf("Command > ");
            fflush(stdout);
            tick_counter = 0;
        } else {
            // Check if song finished naturally
            if (audio_check_at_end()) {
                current_index = (current_index + 1) % my_playlist.count;
                audio_play_track(my_playlist.tracks[current_index].filepath);
                is_playing = 1;
                
                // Full redraw on auto-advance
                tui_draw_dashboard(&my_playlist, current_index, is_playing, audio_get_cursor_seconds(), audio_get_length_seconds());
                printf("Command > ");
                fflush(stdout);
                tick_counter = 0;
            } else if (is_playing) {
                // Quiet background progress update every ~1 second (no full screen wipe!)
                tick_counter++;
                if (tick_counter >= 10) {
                    tui_update_status(audio_get_cursor_seconds(), audio_get_length_seconds());
                    tick_counter = 0;
                }
            }
        }
    }    
}