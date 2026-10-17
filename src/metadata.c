#include <stdio.h>
#include <string.h>
#include "playlist.h"

void extract_metadata(const char *filepath, Track *track){
    // save the full filepath
    snprintf(track->filepath, MAX_PATH_LEN, "%s", filepath);

    // Fallback defualts using file names
    const char *basename = strrchr(filepath, '/');
    basename = basename ? basename + 1 : filepath;

    snprintf(track->filename, sizeof(track->filename), "%s", basename);
    snprintf(track->title, sizeof(track->title), "%s", basename);
    snprintf(track->artist, sizeof(track->artist), "Unknown Artist");

    // open file in binary and check if ID3 tag at the end
    FILE *f = fopen(filepath, "rb");
    if(!f) return;

    // ID3 is exactly 128bytes at the end of the file
    if (fseek(f, -128, SEEK_END) == 0){
        char buffer[128];
        if (fread(buffer, 1, 128, f) == 128){
            // check tag magic header
            if (memcmp(buffer, "TAG", 3) == 0){
                char title_buf[31];
                char artist_buf[31];

                // extract title {30bytes starting at offset 3}
                memcpy(title_buf, buffer + 3, 30);
                title_buf[30] = '\0';
                
                // extract title {30bytes starting at offset 33}
                memcpy(artist_buf, buffer + 33, 30);
                artist_buf[30] = '\0';

                // trim trailing space
                for (int i = 29; i >= 0; i--){
                    if (title_buf[i] == ' ' || title_buf[i] == '\0') title_buf[i] = '\0';
                    else break;
                }
                for(int i = 29; i >= 0; i--){
                    if(artist_buf[i] == ' ' || artist_buf[i] == '\0') artist_buf[i] = '\0';
                    else break;
                }

                // save to track struct if valid
                if (title_buf[0] != '\0'){
                    snprintf(track->title, sizeof(track->title), "%s", title_buf);
                }
                if (artist_buf[0] != '\0'){
                    snprintf(track->artist, sizeof(track->artist), "%s", artist_buf);
                }                
            }
        }
    }
    fclose(f);
}