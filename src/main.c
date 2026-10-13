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

    
    // setup poll struchture to monitor sandard input
    struct pollfd pfd;
    pfd.fd = 0;
    pfd.events = POLLIN;
    
    // Draw tui dasboard
    tui_draw_dashbard(&my_playlist, current_index, is_playing);
    
    while (1) {
        // poll stdin with 100 timout so we can check playback status in background
        int ret = poll(&pfd, 1, 100);

        if (ret > 0) {

            char command;
            scanf(" %c", &command);

            if (command == 'p') {
                audio_pause_track();
                is_playing = 0;
            } else if (command == 'r') {
                audio_resume_track();
                is_playing = 1;
            } else if (command == 'n') {
                current_index = (current_index + 1) % my_playlist.count;
                printf("\nSwitching to track [%d].... \n", current_index + 1);
                audio_play_track(my_playlist.tracks[current_index].filepath);
                is_playing = 1;
            } else if (command == 'q') {
                printf(CLEAR_SCREEN MOVE_HOME);
                printf("Quiting....\n");
                break;
            } else if (command == 'b') {
                current_index = (current_index - 1 + my_playlist.count) % my_playlist.count;
                printf("\nSwitching to track [%d].... \n", current_index + 1);
                audio_play_track(my_playlist.tracks[current_index].filepath);
                is_playing = 1;
            } else {
                printf("\nUnknown command Use: [p]ause, [r]esume, [n]ext, [b]back [q]uit\n");
            }
            printf("command > ");
            fflush(stdout);

            // redraw dashoard after every user action
            tui_draw_dashbard(&my_playlist, current_index, is_playing);
        } else {
            // No user input?....then check if the song finished naturally
            if (audio_check_at_end()) {
                current_index = (current_index + 1) % my_playlist.count;
                printf("\n[Track Finished] Auto advancing to next track[%d]....\n", current_index + 1);
                audio_play_track(my_playlist.tracks[current_index].filepath);
                is_playing = 1;

                // redraw dashboard on every auto advance
                tui_draw_dashbard(&my_playlist, current_index, is_playing);
            }
        }
    }

    // Clean up memory
    audio_engine_uninit();
    Playlist_free(&my_playlist);
    printf("Exiting the player core cleanly.\n");

    return 0;
}