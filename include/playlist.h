#ifndef PLATLIST_H
#define PLATLIST_H

#define MAX_PATH_LEN 512

// represents a single audio track
typedef struct{
    char filepath[MAX_PATH_LEN];
    char filename[256];
} Track;

// represents in memory que/playlist
typedef struct{
    Track *tracks;
    int count;
    int capacity;
} Playlist;

// playlist management function
void playlist_init(Playlist *pl, int initial_capacity);
void playlist_add(Playlist *pl, const char *filepath, const char *filename);
void Playlist_free(Playlist *pl);

#endif