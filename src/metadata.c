#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "playlist.h"

/**
 * Helper function to decode ID3v2 synchsafe integers.
 * ID3v2 stores tag sizes using only the lower 7 bits of each byte 
 * to avoid accidental synchronization markers in the data stream.
 */
static unsigned int parse_synchsafe(const unsigned char *buf) {
    return ((buf[0] & 0x7F) << 21) |
           ((buf[1] & 0x7F) << 14) |
           ((buf[2] & 0x7F) << 7)  |
           (buf[3] & 0x7F);
}

void extract_metadata(const char *filepath, Track *track) {
    // Initialize default track paths and fields
    snprintf(track->filepath, MAX_PATH_LEN, "%s", filepath);
    track->cover_path[0] = '\0'; // Default to empty cover art

    // Generate a unique cache path for this specific track using an incrementing counter
    static int cover_counter = 0;
    char cache_path[MAX_PATH_LEN];
    snprintf(cache_path, sizeof(cache_path), "/tmp/music_player_cover_%d.jpg", cover_counter++);

    // Fallback defaults using the filename from the path
    const char *basename = strrchr(filepath, '/');
    basename = basename ? basename + 1 : filepath;

    snprintf(track->filename, sizeof(track->filename), "%s", basename);
    snprintf(track->title, sizeof(track->title), "%s", basename);
    snprintf(track->artist, sizeof(track->artist), "Unknown Artist");

    // Open the music file in binary read mode
    FILE *f = fopen(filepath, "rb");
    if (!f) return;

    // --- SECTION 1: PARSE ID3v2 TAGS & EMBEDDED COVER ART ---
    char header[10];
    if (fread(header, 1, 10, f) == 10) {
        if (memcmp(header, "ID3", 3) == 0) {
            unsigned int tag_size = parse_synchsafe((unsigned char *)&header[6]);
            int v_major = header[3];
            unsigned char flags = header[5];
            int has_extended_header = (flags & 0x40) != 0;
            
            unsigned char *tag_data = malloc(tag_size);
            if (tag_data) {
                if (fread(tag_data, 1, tag_size, f) == tag_size) {
                    unsigned int cursor = 0;

                    if (has_extended_header && tag_size > 4) {
                        unsigned int ext_size = 0;
                        if (v_major == 3) {
                            unsigned char *es = tag_data;
                            ext_size = (es[0] << 24) | (es[1] << 16) | (es[2] << 8) | es[3];
                            cursor += ext_size + 4;
                        } else if (v_major == 4) {
                            unsigned char *es = tag_data;
                            ext_size = ((es[0] & 0x7F) << 21) | ((es[1] & 0x7F) << 14) | ((es[2] & 0x7F) << 7) | (es[3] & 0x7F);
                            cursor += ext_size;
                        }
                    }
                    
                    while (cursor < tag_size - (v_major == 2 ? 6 : 10)) {
                        char frame_id[5] = {0};
                        unsigned int frame_size = 0;
                        unsigned int header_len = 10;

                        if (v_major == 2) {
                            memcpy(frame_id, tag_data + cursor, 3);
                            unsigned char *fs = tag_data + cursor + 3;
                            frame_size = (fs[0] << 16) | (fs[1] << 8) | fs[2];
                            header_len = 6;
                        } else if (v_major == 3) {
                            memcpy(frame_id, tag_data + cursor, 4);
                            unsigned char *fs = tag_data + cursor + 4;
                            frame_size = (fs[0] << 24) | (fs[1] << 16) | (fs[2] << 8) | fs[3];
                            header_len = 10;
                        } else {
                            memcpy(frame_id, tag_data + cursor, 4);
                            unsigned char *fs = tag_data + cursor + 4;
                            frame_size = ((fs[0] & 0x7F) << 21) | ((fs[1] & 0x7F) << 14) | ((fs[2] & 0x7F) << 7) | (fs[3] & 0x7F);
                            header_len = 10;
                        }

                        if (frame_id[0] == '\0' || frame_size == 0 || cursor + header_len + frame_size > tag_size) break;

                        if (strcmp(frame_id, "APIC") == 0 || strcmp(frame_id, "PIC") == 0) {
                            unsigned char *apic_payload = tag_data + cursor + header_len;
                            unsigned int apic_len = frame_size;
                            
                            unsigned int img_offset = 0;
                            for (unsigned int i = 0; i < apic_len - 3; i++) {
                                if (apic_payload[i] == 0xFF && apic_payload[i+1] == 0xD8 && apic_payload[i+2] == 0xFF) {
                                    img_offset = i;
                                    break;
                                }
                                if (apic_payload[i] == 0x89 && apic_payload[i+1] == 0x50 && apic_payload[i+2] == 0x4E && apic_payload[i+3] == 0x47) {
                                    img_offset = i;
                                    break;
                                }
                            }

                            if (img_offset > 0 && img_offset < apic_len) {
                                FILE *img_out = fopen(cache_path, "wb");
                                if (img_out) {
                                    fwrite(apic_payload + img_offset, 1, apic_len - img_offset, img_out);
                                    fclose(img_out);
                                    
                                    snprintf(track->cover_path, sizeof(track->cover_path), "%s", cache_path);
                                    break;
                                }
                            }
                        }

                        cursor += header_len + frame_size;
                    }
                }
                free(tag_data);
            }
        }
    }

    // --- SECTION 2: PARSE ID3v1 TEXT TAGS ---
    if (fseek(f, -128, SEEK_END) == 0) {
        char buffer[128];
        if (fread(buffer, 1, 128, f) == 128) {
            if (memcmp(buffer, "TAG", 3) == 0) {
                char title_buf[31];
                char artist_buf[31];

                memcpy(title_buf, buffer + 3, 30);
                title_buf[30] = '\0';
                memcpy(artist_buf, buffer + 33, 30);
                artist_buf[30] = '\0';

                for (int i = 29; i >= 0; i--) {
                    if (title_buf[i] == ' ' || title_buf[i] == '\0') title_buf[i] = '\0';
                    else break;
                }
                for (int i = 29; i >= 0; i--) {
                    if (artist_buf[i] == ' ' || artist_buf[i] == '\0') artist_buf[i] = '\0';
                    else break;
                }

                if (title_buf[0] != '\0') snprintf(track->title, sizeof(track->title), "%s", title_buf);
                if (artist_buf[0] != '\0') snprintf(track->artist, sizeof(track->artist), "%s", artist_buf);
            }
        }
    }
    
    fclose(f);

    // --- SECTION 3: ROBUST FFMPEG FALLBACK ---
    if (track->cover_path[0] == '\0') {
        char cmd[MAX_PATH_LEN * 2 + 128];
        snprintf(cmd, sizeof(cmd), "ffmpeg -y -i \"%s\" -an -vcodec copy \"%s\" 2>/dev/null", filepath, cache_path);
        system(cmd);
        
        FILE *test_img = fopen(cache_path, "rb");
        if (test_img) {
            fseek(test_img, 0, SEEK_END);
            long size = ftell(test_img);
            fclose(test_img);
            
            if (size > 1000) {
                snprintf(track->cover_path, sizeof(track->cover_path), "%s", cache_path);
            }
        }
    }
}