#define _DEFAULT_SOURCE
#include "data_manager.h"
#include "video_list.h"
#include "globals.h" // Use the centralized global declarations
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE ".mirava_data.json"

void load_data_from_json()
{
    json_t *root;
    json_error_t error;

    root = json_load_file(DATA_FILE, 0, &error);
    if (!root)
        return;

    json_t *name_obj = json_object_get(root, "course_name");
    if (json_is_string(name_obj))
    {
        free(g_course_name);
        g_course_name = strdup(json_string_value(name_obj));
    }

    json_t *videos_array = json_object_get(root, "videos");
    if (json_is_array(videos_array))
    {
        size_t index;
        json_t *value;
        json_array_foreach(videos_array, index, value)
        {
            const char *path = json_string_value(json_object_get(value, "path"));
            json_int_t duration = json_integer_value(json_object_get(value, "duration_sec"));
            json_int_t watched = json_integer_value(json_object_get(value, "watched_sec"));

            if (path)
            {
                VideoInfo *vid = malloc(sizeof(VideoInfo));
                if (vid)
                {
                    vid->path = strdup(path);
                    vid->duration_sec = duration;
                    vid->watched_sec = watched;
                    vid->found_on_disk = 0;
                    add_video_to_list(vid);
                }
            }
        }
    }
    json_decref(root);
}

void save_data_to_json()
{
    json_t *root = json_object();
    json_object_set_new(root, "course_name", json_string(g_course_name ? g_course_name : ""));

    json_t *videos_array = json_array();
    for (size_t i = 0; i < g_video_count; i++)
    {
        VideoInfo *vid = g_video_list[i];
        json_t *video_obj = json_pack("{s:s, s:I, s:I}",
                                      "path", vid->path,
                                      "duration_sec", (json_int_t)vid->duration_sec,
                                      "watched_sec", (json_int_t)vid->watched_sec);
        if (video_obj)
        {
            json_array_append_new(videos_array, video_obj);
        }
    }
    json_object_set_new(root, "videos", videos_array);

    if (json_dump_file(root, DATA_FILE, JSON_INDENT(2)) != 0)
    {
        fprintf(stderr, "Error: Failed to write to JSON file '%s'.\n", DATA_FILE);
    }
    json_decref(root);
}
