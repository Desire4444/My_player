#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "playlist.h"
#include "audio.h"
#include "server.h"

int main(int argc, char *argv[]){
    printf("Initailizing local music web server core......\n");

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

    // Initialize audio engine
    if (!audio_engine_init()) {
        Playlist_free(&my_playlist);
        return 0;
    }

    // start web server (runs main server loop)
    start_web_server(&my_playlist, "http://0.0.0.0:8080");

    // cleanup (not needed but just my practice as usual)
    audio_engine_uninit();
    Playlist_free(&my_playlist);

    return 0;
}