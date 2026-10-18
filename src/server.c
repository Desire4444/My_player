#include <stdio.h>
#include <stdlib.h>
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
    // 2. API Endpoint: Serve Full Playlist JSON Array
    else if (mg_match(hm->uri, mg_str("/api/playlist"), NULL)) {
        char *json_body = playlist_to_json(current_playlist);
        mg_http_reply(c, 200, "Content-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\n", "%s", json_body);
        free(json_body);
    }
    // 3. API Endpoint: Direct Track Selection or Play
    else if (mg_match(hm->uri, mg_str("/api/play"), NULL)) {
        char query_buf[32];
        if (mg_http_get_var(&hm->query, "index", query_buf, sizeof(query_buf)) > 0) {
            int requested_index = atoi(query_buf);
            if (requested_index >= 0 && requested_index < current_playlist->count) {
                current_track_index = requested_index;
                audio_play_track(current_playlist->tracks[current_track_index].filepath);
                is_playing = 1;
            }
        } else {
            audio_resume_track();
            is_playing = 1;
        }
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"ok\"}");
    }
    // 4. API Endpoint: Play / Pause
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
    // 5. API Endpoint: Next Track
    else if (mg_match(hm->uri, mg_str("/api/next"), NULL)) {
        if (current_playlist->count > 0) {
            current_track_index = (current_track_index + 1) % current_playlist->count;
            audio_play_track(current_playlist->tracks[current_track_index].filepath);
            is_playing = 1;
        }
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"next\"}");
    }
    // 6. API Endpoint: Previous Track
    else if (mg_match(hm->uri, mg_str("/api/prev"), NULL)) {
        if (current_playlist->count > 0) {
            current_track_index = (current_track_index - 1 + current_playlist->count) % current_playlist->count;
            audio_play_track(current_playlist->tracks[current_track_index].filepath);
            is_playing = 1;
        }
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"prev\"}");
    }
    // 7. API Endpoint: Set Volume
    else if (mg_match(hm->uri, mg_str("/api/volume"), NULL)) {
        char query_buf[32];
        if (mg_http_get_var(&hm->query, "level", query_buf, sizeof(query_buf)) > 0) {
            float vol_percent = atof(query_buf);
            // Convert percentage (0-100) to multiplier (0.0 - 1.0)
            audio_set_volume(vol_percent / 100.0f);
        }
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"volume_updated\"}");
    }
    // 8. API Endpoint: Serve Cover Art for Current Track
    else if (mg_match(hm->uri, mg_str("/api/cover"), NULL)) {
        Track *current = (current_playlist && current_playlist->count > 0) 
                         ? &current_playlist->tracks[current_track_index] 
                         : NULL;
                         
        if (current) {
            printf("[Server DEBUG] /api/cover -> Track: '%s' | Cover Path: '%s'\n", current->title, current->cover_path);
        } else {
            printf("[Server DEBUG] /api/cover -> current track is NULL!\n");
        }

        if (current && current->cover_path[0] != '\0') {
            mg_http_serve_file(c, hm, current->cover_path, NULL);
        } else {
            mg_http_reply(c, 404, "", "No cover");
        }
        return;
    }
    // 9. Serve Static Files (HTML/CSS/JS from web/ directory)
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