#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "scanner.h"
#include "playlist.h"
#include "metadata.h"

int is_audio_file(const char *filename){
    const char *ext = strrchr(filename, '.');
    if(!ext) return 0;

    if (strcmp(ext, ".mp3") == 0 ||
        strcmp(ext, ".flac") == 0 ||
        strcmp(ext, ".wav") == 0){
            return 1;
    }
    return 0;
}

void scan_music_directory_recursive(const char *base_path, Playlist *pl){
    DIR *dir = opendir(base_path);
    if (!dir) {
        perror("Error openning music directory");
        return; // Skip folders we dont have permission to.
    }

    struct dirent *entry;
    char path[MAX_PATH_LEN];
    
    while ((entry = readdir(dir)) != NULL) {
        // skipping hidden file directories like(. and ..)
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        
        // BUild the full path to entry
        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        struct stat path_stat;
        if (stat(path, &path_stat) != 0) {
            continue;
        }

        else if (S_ISREG(path_stat.st_mode)){
            // It's a regular file? then check if its music
            if (is_audio_file(entry->d_name)){
                // create a temporary track struct extract tags then add it
                Track new_track;
                extract_metadata(path, &new_track);
                playlist_add(pl, new_track.filepath, new_track.filename, new_track.title, new_track.artist, new_track.cover_path); // <-- Added new_track.cover_path here
            }
        }
        
    }
    closedir(dir);
}