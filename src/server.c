#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include "mongoose.h"
#include "server.h"
#include "audio.h"
#include "scanner.h"

static Playlist *current_playlist = NULL;
static int current_track_index = 0;
static int is_playing = 1;

// Helper function to escape JSON strings
static void json_escape(const char *src, char *dest, size_t dest_size) {
size_t j = 0;
for (size_t i = 0; src[i] != '\0' && j < dest_size - 2; i++) {
if (src[i] == '"' || src[i] == '\\') {
dest[j++] = '\\';
} else if (src[i] == '\n') {
dest[j++] = ' ';
continue;
}
dest[j++] = src[i];
}
dest[j] = '\0';
}

static void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
if (ev == MG_EV_HTTP_MSG) {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;

    // 1. API Endpoint: Get Player Status (JSON)
    if (mg_match(hm->uri, mg_str("/api/status"), NULL)) {
        char json_buf[1024];
        const Track *t = (current_playlist && current_playlist->count > 0) ? &current_playlist->tracks[current_track_index] : NULL;
        
        float cur_time = audio_get_cursor_seconds();
        float tot_time = audio_get_length_seconds();
        float volume = audio_get_volume() * 100.0f;

        snprintf(json_buf, sizeof(json_buf),
            "{\"title\":\"%s\", \"artist\":\"%s\", \"current\":%d, \"total\":%d, \"playing\":%d, \"cur_time\":%.1f, \"tot_time\":%.1f, \"volume\":%.1f}",
            t ? t->title : "None",
            t ? t->artist : "Unknown",
            current_track_index + 1,
            current_playlist ? current_playlist->count : 0,
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
            if (current_playlist && requested_index >= 0 && requested_index < current_playlist->count) {
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
    // 4. API Endpoint: Pause / Resume
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
        if (current_playlist && current_playlist->count > 0) {
            current_track_index = (current_track_index + 1) % current_playlist->count;
            audio_play_track(current_playlist->tracks[current_track_index].filepath);
            is_playing = 1;
        }
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"next\"}");
    }
    // 6. API Endpoint: Previous Track
    else if (mg_match(hm->uri, mg_str("/api/prev"), NULL)) {
        if (current_playlist && current_playlist->count > 0) {
            current_track_index = (current_track_index - 1 + current_playlist->count) % current_playlist->count;
            audio_play_track(current_playlist->tracks[current_track_index].filepath);
            is_playing = 1;
        }
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"prev\"}");
    }
    // 7. API Endpoint: Seek to Timestamp
    else if (mg_match(hm->uri, mg_str("/api/seek"), NULL)) {
        char query_buf[32];
        if (mg_http_get_var(&hm->query, "pos", query_buf, sizeof(query_buf)) > 0) {
            float target_seconds = atof(query_buf);
            audio_seek_to_seconds(target_seconds);
        }
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"seek_updated\"}");
    }
    // 8. API Endpoint: Set Volume
    else if (mg_match(hm->uri, mg_str("/api/volume"), NULL)) {
        char query_buf[32];
        if (mg_http_get_var(&hm->query, "level", query_buf, sizeof(query_buf)) > 0) {
            float vol_percent = atof(query_buf);
            audio_set_volume(vol_percent / 100.0f);
        }
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"volume_updated\"}");
    }
    // 9. API Endpoint: Serve Cover Art
    else if (mg_match(hm->uri, mg_str("/api/cover"), NULL)) {
        Track *current = (current_playlist && current_playlist->count > 0) 
                         ? &current_playlist->tracks[current_track_index] 
                         : NULL;

        if (current && current->cover_path[0] != '\0') {
            mg_http_serve_file(c, hm, current->cover_path, NULL);
        } else {
            mg_http_reply(c, 404, "", "No cover");
        }
        return;
    }
    // 10. API Endpoint: Directory Browser List
    else if (mg_match(hm->uri, mg_str("/api/browse"), NULL)) {
        char path_buf[512] = {0};
        if (mg_http_get_var(&hm->query, "path", path_buf, sizeof(path_buf)) <= 0 || strlen(path_buf) == 0) {
            const char *home = getenv("HOME");
            snprintf(path_buf, sizeof(path_buf), "%s", home ? home : "/");
        }

        DIR *dir = opendir(path_buf);
        if (!dir) {
            mg_http_reply(c, 400, "Content-Type: application/json\r\n", "{\"error\":\"Invalid directory\"}");
            return;
        }

        size_t buf_size = 16384;
        char *json = malloc(buf_size);
        if (!json) {
            closedir(dir);
            mg_http_reply(c, 500, "", "Out of memory");
            return;
        }

        char esc_path[1024];
        json_escape(path_buf, esc_path, sizeof(esc_path));

        size_t offset = snprintf(json, buf_size, "{\"current_path\":\"%s\", \"folders\":[", esc_path);

        struct dirent *entry;
        int first = 1;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            char full_item_path[1024];
            snprintf(full_item_path, sizeof(full_item_path), "%s/%s", path_buf, entry->d_name);

            struct stat st;
            if (stat(full_item_path, &st) == 0 && S_ISDIR(st.st_mode)) {
                char esc_name[512];
                json_escape(entry->d_name, esc_name, sizeof(esc_name));

                offset += snprintf(json + offset, buf_size - offset,
                    "%s{\"name\":\"%s\", \"path\":\"%s\"}",
                    first ? "" : ",", esc_name, full_item_path);
                first = 0;
            }
        }
        closedir(dir);
        snprintf(json + offset, buf_size - offset, "]}");

        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "%s", json);
        free(json);
    }
    // 11. API Endpoint: Load & Play Selected Folder
    else if (mg_match(hm->uri, mg_str("/api/select_folder"), NULL)) {
        char path_buf[512] = {0};
        if (mg_http_get_var(&hm->query, "path", path_buf, sizeof(path_buf)) > 0) {
            audio_stop_track();

            Playlist_free(current_playlist);
            playlist_init(current_playlist, 10);

            scan_music_directory_recursive(path_buf, current_playlist);

            current_track_index = 0;
            if (current_playlist->count > 0) {
                audio_play_track(current_playlist->tracks[0].filepath);
                is_playing = 1;
            } else {
                is_playing = 0;
            }
        }
        mg_http_reply(c, 200, "Content-Type: application/json\r\n", "{\"status\":\"ok\", \"count\":%d}", current_playlist->count);
    }
    // 12. Serve Static Files
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

printf("\n[Web Server] Starting HTTP server at %s ...\n", address);
if (mg_http_listen(&mgr, address, ev_handler, NULL) == NULL) {
    fprintf(stderr, "Failed to listen on %s\n", address);
    return;
}

if (current_playlist && current_playlist->count > 0) {
    audio_play_track(current_playlist->tracks[0].filepath);
}

while (1) {
    mg_mgr_poll(&mgr, 100);

    if (is_playing && current_playlist && current_playlist->count > 0 && audio_check_at_end()) {
        current_track_index = (current_track_index + 1) % current_playlist->count;
        audio_play_track(current_playlist->tracks[current_track_index].filepath);
    }
}
mg_mgr_free(&mgr);
}