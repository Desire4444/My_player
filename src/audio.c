#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <sys/stat.h>
#include <stdio.h>
#include "audio.h"

static ma_engine engine;
static ma_sound sound;
static int engine_initialized = 0;

int audio_engine_init(void) {
    ma_result result = ma_engine_init(NULL, &engine);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to initialize audio engine (Error: %d)\n", result);
        return 0;
    }
    engine_initialized = 1;
    return 1;
}

int audio_play_track(const char *filepath){
    if (!engine_initialized) {
        fprintf(stderr,"Audio engine not initialized. \n");
        return 0;
    }

    // if a sound is already loaded/playing uninit first
    ma_sound_uninit(&sound);

    // load audio into miniaudio sound object
    ma_result result = ma_sound_init_from_file(&engine, filepath, 0, NULL, NULL, &sound);
    if (result != MA_SUCCESS) {
        fprintf(stderr, "Failed to load track: %s (Error: %d)\n", filepath, result);
        return 0;
    }

    // for outputting the sound
    ma_sound_start(&sound);

    // start playback
    printf("\n[Now Playing] -> %s\n", filepath);
    printf("Press Ctrl + C to stop.\n");

    return 1;
}

void audio_engine_uninit(void){
    if (engine_initialized) {
        ma_sound_uninit(&sound);
        ma_engine_uninit(&engine);
        engine_initialized = 0;
    }
}

void audio_pause_track(void){
    ma_sound_stop(&sound);
    printf("\n[paused]\n");
}

void audio_resume_track(void){
    ma_sound_start(&sound);
    printf("\n[Resumed]\n");
}