#include <stdio.h>
#include "scanner.h"
#include "playlist.h"

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
    for (int i = 0; i < my_playlist.count; i++) {
        printf("[%d] %s\n    path: %s\n", i + 1, my_playlist.tracks[i].filename, my_playlist.tracks[i].filepath);
    }

    // Clean up memory
    Playlist_free(&my_playlist);
    return 0;
}