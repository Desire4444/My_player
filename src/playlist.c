#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "playlist.h"

void playlist_init(Playlist *pl, int initial_capacity){
    pl->tracks = malloc(initial_capacity * sizeof(Track));
    pl->count = 0;
    pl->capacity = initial_capacity;
}

void playlist_add(Playlist *pl, const char *filepath, const char *filename){
    // resize array if its full
    if (pl->count >= pl->capacity) {
        pl->capacity *= 2;
        Track *temp = realloc(pl->tracks, pl->capacity * sizeof(Track));
        if (!temp) {
            perror("Failed to expand playlist memory");
            return;
        }
        pl->tracks = temp;    
    }

    // copy data into the track struct
    snprintf(pl->tracks[pl->count].filepath, MAX_PATH_LEN, "%s" , filepath);
    snprintf(pl->tracks[pl->count].filename, 256, "%s", filename);
    pl->count++;
}

void Playlist_free(Playlist *pl){
    free(pl->tracks);
    pl->tracks = NULL;
    pl->count = 0;
    pl->capacity = 0;
}