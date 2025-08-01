#include "video_list.h"
#include "globals.h" // Include the global declarations
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// --- Global Variables DEFINED here. This is the one and only place. ---
char *g_course_name = NULL;
VideoInfo **g_video_list = NULL;
size_t g_video_count = 0;
size_t g_video_capacity = 0;

void add_video_to_list(VideoInfo *video)
{
    if (g_video_count >= g_video_capacity)
    {
        size_t new_capacity = g_video_capacity == 0 ? 16 : g_video_capacity * 2;
        VideoInfo **new_list = realloc(g_video_list, new_capacity * sizeof(VideoInfo *));
        if (!new_list)
        {
            fprintf(stderr, "Error: Failed to allocate memory for video list.\n");
            return;
        }
        g_video_list = new_list;
        g_video_capacity = new_capacity;
    }
    g_video_list[g_video_count++] = video;
}

VideoInfo *find_video_by_path(const char *path)
{
    // Add a sanity check to prevent crash if list is NULL
    if (!g_video_list || !path)
    {
        return NULL;
    }
    for (size_t i = 0; i < g_video_count; i++)
    {
        if (g_video_list[i] && g_video_list[i]->path && strcmp(g_video_list[i]->path, path) == 0)
        {
            return g_video_list[i];
        }
    }
    return NULL;
}

void prune_missing_videos()
{
    if (g_video_count == 0)
        return;

    size_t new_count = 0;
    for (size_t i = 0; i < g_video_count; i++)
    {
        if (g_video_list[i]->found_on_disk)
        {
            if (i != new_count)
            {
                g_video_list[new_count] = g_video_list[i];
            }
            new_count++;
        }
        else
        {
            free(g_video_list[i]->path);
            free(g_video_list[i]);
        }
    }
    g_video_count = new_count;
}

void cleanup_video_list()
{
    if (g_video_list)
    {
        for (size_t i = 0; i < g_video_count; i++)
        {
            free(g_video_list[i]->path);
            free(g_video_list[i]);
        }
        free(g_video_list);
        g_video_list = NULL;
        g_video_count = 0;
        g_video_capacity = 0;
    }
}
