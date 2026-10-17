#ifndef METADAT_H
#define METADAT_H

#include "playlist.h"

// Extracts ID3 tags or fallback defualts for the track
void extract_metadata(const char *filepath, Track *track);

#endif