#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

// Audio engine lifecycle
bool audio_engine_init(void);
void audio_engine_uninit(void);

// Playback control functions
bool audio_play_track(const char *filepath);
void audio_pause_track(void);
void audio_resume_track(void);
void audio_stop_track(void);

// Volume and Seeking
void audio_set_volume(float volume); // 0.0f to 1.0f
float audio_get_volume(void);
void audio_seek_to_seconds(float seconds);

// Time & Status queries
float audio_get_cursor_seconds(void);
float audio_get_length_seconds(void);
bool audio_check_at_end(void);

#endif