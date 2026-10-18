#ifndef SERVER_H
#define SERVER_H

#include "playlist.h"

// start mongoose http web server loop
void start_web_server(Playlist *pl, const char *address);

#endif