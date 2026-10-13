#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include "scanner.h"
#include "playlist.h"

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

        if (S_ISDIR(path_stat.st_mode)) {
            scan_music_directory_recursive(path, pl);
            // It's a directory? then dive into it recursively.......
        } else if (S_ISREG(path_stat.st_mode)) {
            // It's a file? check if its a music file.........
            if (is_audio_file(entry->d_name)) {
                playlist_add(pl, path, entry->d_name);
            } 
        }
    }
    closedir(dir);
}