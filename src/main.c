#include <stdio.h>
#include <stdlib.h>
#include "scanner.h"
#include "playlist.h"
#include "audio.h"
#include "server.h"

int main(int argc, char *argv[]){
    printf("Initializing local music web server core......\n");

    // Allow one optional argument: use argv[1] if provided, otherwise default to "."
    const char *music_path = (argc > 1) ? argv[1] : ".";

    // Initialize our playlist queue with a starting capacity of 10[cite: 1]
    Playlist my_playlist;
    playlist_init(&my_playlist, 10);

    printf("Scanning directory (Including subfolders): %s\n", music_path);
    scan_music_directory_recursive(music_path, &my_playlist);

    printf("-----Playlist Loaded-----\n");
    printf("Number of tracks found: %d\n", my_playlist.count);

    if (my_playlist.count == 0) {
        printf("No music tracks found in this directory. Starting server with an empty playlist.\n");
    }

    // Initialize audio engine[cite: 1]
    if (!audio_engine_init()) {
        Playlist_free(&my_playlist);
        return 0;
    }

    // start web server (runs main server loop)[cite: 1]
    start_web_server(&my_playlist, "http://0.0.0.0:8080");

    // cleanup (not needed but just my practice as usual)[cite: 1]
    audio_engine_uninit();
    Playlist_free(&my_playlist);

    return 0;
}