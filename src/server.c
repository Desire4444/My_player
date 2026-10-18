#include <stdio.h>
#include <string.h>
#include "mongoose.h"
#include "server.h"
#include "audio.h"

// Global pointer so our event handler can access the playlist and state
static Playlist *current_playlist = NULL;
static int current_track_index = 0;
static int is_playing = 1;

static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

        // 1. API Endpoint: Get Player Status (JSON)
        if (mg_match(hm->uri, mg_str("/api/status"), NULL)) {
            char json_buf[1024];
            const Track *t = (current_playlist->count > 0) ? &current_playlist->tracks[current_track_index] : NULL;
            
            float cur_time = audio_get_cursor_seconds();
            float tot_time = audio_get_length_seconds();
            float volume = audio_get_volume() * 100.0f;

            snprintf(json_buf, sizeof(json_buf),
                "{\"title\":\"%s\", \"artist\":\"%s\", \"current\":%d, \"total\":%d, \"playing\":%d, \"cur_time\":%.1f, \"tot_time\":%.1f, \"volume\":%.1f}",
                t ? t->title : "None",
                t ? t->artist : "Unknown",
                current_track_index + 1,
                current_playlist->count,
                is_playing,
                cur_time,
                tot_time,
                volume
            );

            mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", json_buf);
        }
        // 2. API Endpoint: Play / Pause
        else if (mg_match(hm->uri, mg_str("/api/pause"), NULL)) {
            audio_pause_track();
            is_playing = 0;
            mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"paused\"}");
        }
        else if (mg_match(hm->uri, mg_str("/api/resume"), NULL)) {
            audio_resume_track();
            is_playing = 1;
            mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"resumed\"}");
        }
        // 3. API Endpoint: Next Track
        else if (mg_match(hm->uri, mg_str("/api/next"), NULL)) {
            if (current_playlist->count > 0) {
                current_track_index = (current_track_index + 1) % current_playlist->count;
                audio_play_track(current_playlist->tracks[current_track_index].filepath);
                is_playing = 1;
            }
            mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"next\"}");
        }
        // 4. API Endpoint: Previous Track
        else if (mg_match(hm->uri, mg_str("/api/prev"), NULL)) {
            if (current_playlist->count > 0) {
                current_track_index = (current_track_index - 1 + current_playlist->count) % current_playlist->count;
                audio_play_track(current_playlist->tracks[current_track_index].filepath);
                is_playing = 1;
            }
            mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"prev\"}");
        }
        // 5. Serve Static Files (HTML/CSS/JS from web/ directory)
        else {
            struct mg_http_serve_opts opts = { .root_dir = "web" };
            mg_http_serve_dir(c, hm, &opts);
        }
    }
}

void start_web_server(Playlist *pl, const char *address) {
    current_playlist = pl;
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);

    printf("\n[Web Server] Starting HTTP server at http://localhost:8080 ...\n");
    if (mg_http_listen(&mgr, address, ev_handler, NULL) == NULL) {
        fprintf(stderr, "Failed to listen on %s\n", address);
        return;
    }

    // Start playing the first track automatically if playlist isn't empty
    if (current_playlist->count > 0) {
        audio_play_track(current_playlist->tracks[0].filepath);
    }

    // Run event loop
    while (1) {
        mg_mgr_poll(&mgr, 100); // 100ms poll interval

        // Auto-advance check for when songs end naturally
        if (is_playing && audio_check_at_end()) {
            current_track_index = (current_track_index + 1) % current_playlist->count;
            audio_play_track(current_playlist->tracks[current_track_index].filepath);
        }
    }

    mg_mgr_free(&mgr);
}