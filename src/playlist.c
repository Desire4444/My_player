#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "playlist.h"

void playlist_init(Playlist *pl, int initial_capacity){
    pl->tracks = malloc(initial_capacity * sizeof(Track));
    pl->count = 0;
    pl->capacity = initial_capacity;
    pl->current_index = 0;
}

void playlist_add(Playlist *pl, const char *filepath, const char *filename, const char *title, const char *artist, const char *cover_path){
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

    // copy data into the track struct (including cover_path!)
    snprintf(pl->tracks[pl->count].filepath, MAX_PATH_LEN, "%s" , filepath);
    snprintf(pl->tracks[pl->count].filename, 256, "%s", filename);
    snprintf(pl->tracks[pl->count].title, 128, "%s", title);
    snprintf(pl->tracks[pl->count].artist, 128, "%s", artist);
    snprintf(pl->tracks[pl->count].cover_path, MAX_PATH_LEN, "%s", cover_path); // <-- Added this line
    pl->count++;
}

void Playlist_free(Playlist *pl){
    free(pl->tracks);
    pl->tracks = NULL;
    pl->count = 0;
    pl->capacity = 0;
    pl->current_index = 0;
}

// Helper function to escape special JSON characters
static void json_escape(const char *src, char *dest, size_t dest_size) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\0' && j < dest_size - 2; i++) {
        if (src[i] == '"' || src[i] == '\\') {
            dest[j++] = '\\';
        } else if (src[i] == '\n') {
            dest[j++] = ' ';
            continue;
        }
        dest[j++] = src[i];
        }
        dest[j] = '\0';
    }

/* Serializes the dynamic playlist into a JSON array string */
char *playlist_to_json(const Playlist *pl) {
if (!pl || pl->count == 0) {
return strdup("[]");
}

// Allocate dynamic buffer for JSON
size_t buf_size = (size_t)pl->count * 512 + 128;
char *json = malloc(buf_size);
if (!json) return strdup("[]");

size_t offset = 0;
offset += snprintf(json + offset, buf_size - offset, "[\n");

for (int i = 0; i < pl->count; i++) {
    char esc_title[256] = {0};
    char esc_artist[256] = {0};
    char esc_filename[256] = {0};

    json_escape(pl->tracks[i].title, esc_title, sizeof(esc_title));
    json_escape(pl->tracks[i].artist, esc_artist, sizeof(esc_artist));
    json_escape(pl->tracks[i].filename, esc_filename, sizeof(esc_filename));

    offset += snprintf(json + offset, buf_size - offset,
        "  {\"index\": %d, \"title\": \"%s\", \"artist\": \"%s\", \"filename\": \"%s\"}%s\n",
        i, esc_title, esc_artist, esc_filename,
        (i == pl->count - 1) ? "" : ",");
}

snprintf(json + offset, buf_size - offset, "]");
return json;


}