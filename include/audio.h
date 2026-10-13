#ifndef AUDIO_H
#define AUDIO_H

#include "playlist.h"

// initialize audio engine and output device
int audio_engine_init(void);

// plays a specific track from playlist
int audio_play_track(const char *filepath);

// shutdown audio engine
void audio_engine_uninit(void);

// pause
void audio_pause_track(void);

// resume
void audio_resume_track(void);

// check if track playing has reached its end
int audio_check_at_end(void);

#endif