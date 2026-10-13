
#ifndef SCANNER_H
#define SCANNER_H

#include "playlist.h"

// scans given directory for music files
void scan_music_directory_recursive(const char *base_path, Playlist *pl);

#endif

