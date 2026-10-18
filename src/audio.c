#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "audio.h"

static ma_engine g_engine;
static ma_sound g_sound;
static bool g_engine_initialized = false;
static bool g_sound_initialized = false;

bool audio_engine_init(void) {
ma_result result = ma_engine_init(NULL, &g_engine);
if (result != MA_SUCCESS) {
fprintf(stderr, "[Audio Engine] Failed to initialize miniaudio engine.\n");
return false;
}
g_engine_initialized = true;
return true;
}

void audio_engine_uninit(void) {
if (g_sound_initialized) {
ma_sound_uninit(&g_sound);
g_sound_initialized = false;
}
if (g_engine_initialized) {
ma_engine_uninit(&g_engine);
g_engine_initialized = false;
}
}

bool audio_play_track(const char *filepath) {
if (!g_engine_initialized) return false;

if (g_sound_initialized) {
    ma_sound_uninit(&g_sound);
    g_sound_initialized = false;
}

ma_result result = ma_sound_init_from_file(&g_engine, filepath, MA_SOUND_FLAG_DECODE | MA_SOUND_FLAG_ASYNC, NULL, NULL, &g_sound);
if (result != MA_SUCCESS) {
    fprintf(stderr, "[Audio Engine] Failed to load audio file: %s\n", filepath);
    return false;
}

g_sound_initialized = true;
ma_sound_start(&g_sound);
return true;


}

void audio_pause_track(void) {
if (g_sound_initialized) {
ma_sound_stop(&g_sound);
}
}

void audio_resume_track(void) {
if (g_sound_initialized) {
ma_sound_start(&g_sound);
}
}

void audio_stop_track(void) {
if (g_sound_initialized) {
ma_sound_stop(&g_sound);
ma_sound_seek_to_pcm_frame(&g_sound, 0);
}
}

void audio_set_volume(float volume) {
if (g_sound_initialized) {
if (volume < 0.0f) volume = 0.0f;
if (volume > 1.0f) volume = 1.0f;
ma_sound_set_volume(&g_sound, volume);
}
}

float audio_get_volume(void) {
if (g_sound_initialized) {
return ma_sound_get_volume(&g_sound);
}
return 1.0f;
}

void audio_seek_to_seconds(float seconds) {
if (!g_sound_initialized) return;

ma_uint32 sample_rate = ma_engine_get_sample_rate(&g_engine);
if (sample_rate == 0) sample_rate = 44100;

ma_uint64 target_frame = (ma_uint64)(seconds * (float)sample_rate);
ma_sound_seek_to_pcm_frame(&g_sound, target_frame);


}

float audio_get_cursor_seconds(void) {
if (!g_sound_initialized) return 0.0f;

ma_uint32 sample_rate = ma_engine_get_sample_rate(&g_engine);
if (sample_rate == 0) return 0.0f;

ma_uint64 cursor_frame = 0;
ma_sound_get_cursor_in_pcm_frames(&g_sound, &cursor_frame);
return (float)cursor_frame / (float)sample_rate;


}

float audio_get_length_seconds(void) {
if (!g_sound_initialized) return 0.0f;

float length = 0.0f;
ma_sound_get_length_in_seconds(&g_sound, &length);
return length;


}

bool audio_check_at_end(void) {
if (!g_sound_initialized) return false;
return ma_sound_at_end(&g_sound);
}