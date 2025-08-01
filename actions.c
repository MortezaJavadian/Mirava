#define _DEFAULT_SOURCE
#include "actions.h"
#include "globals.h" // Use the centralized global declarations
#include "cli.h"
#include "data_manager.h"
#include "file_utils.h"
#include "video_list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>

#define DATA_FILE ".mirava_data.json"

// Helper function to get relative path from course root
static const char* get_relative_path(const char *full_path, const char *course_root)
{
    if (!full_path || !course_root) {
        return full_path;
    }
    
    size_t root_len = strlen(course_root);
    
    // If full_path starts with course_root, return the relative part
    if (strncmp(full_path, course_root, root_len) == 0) {
        const char *relative = full_path + root_len;
        // Skip leading slash if present
        if (relative[0] == '/') {
            relative++;
        }
        return relative;
    }
    
    // If it doesn't match, return the original path
    return full_path;
}

// --- Private function for scanning filesystem ---
static void scan_and_sync_videos(const char *basePath)
{
    char full_path[1024];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir)
        return;

    const char *course_root = get_course_root_dir();

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", basePath, dp->d_name);
        
        // Get relative path from course root
        const char *display_path = get_relative_path(full_path, course_root);
        
        // Skip the JSON data file
        if (strcmp(dp->d_name, DATA_FILE) == 0)
            continue;

        struct stat statbuf;
        if (stat(full_path, &statbuf) != -1)
        {
            if (S_ISDIR(statbuf.st_mode))
            {
                scan_and_sync_videos(full_path);
            }
            else if (is_video_file(full_path))
            {
                VideoInfo *existing_video = find_video_by_path(display_path);
                if (existing_video)
                {
                    existing_video->found_on_disk = 1;
                    existing_video->duration_sec = get_duration_in_seconds(full_path);
                }
                else
                {
                    VideoInfo *new_video = malloc(sizeof(VideoInfo));
                    if (new_video)
                    {
                        new_video->path = strdup(display_path);
                        new_video->duration_sec = get_duration_in_seconds(full_path);
                        new_video->watched_sec = 0;
                        new_video->found_on_disk = 1;
                        add_video_to_list(new_video);
                    }
                }
            }
        }
    }
    closedir(dir);
}

void action_list_and_sync()
{
    load_data_from_json();

    if (!g_course_name)
    {
        prompt_for_course_name();
    }

    // Get the course root directory and scan from there
    const char *course_root = get_course_root_dir();
    if (course_root) {
        scan_and_sync_videos(course_root);
    } else {
        scan_and_sync_videos(".");
    }
    
    prune_missing_videos();

    display_video_list();
    save_data_to_json();
    printf("\nData synced and saved successfully.\n");
}

static long long parse_progress_string(const char *progress_str, long long total_duration)
{
    if (strchr(progress_str, '%'))
    {
        int percentage = atoi(progress_str);
        if (percentage >= 0 && percentage <= 100)
            return (total_duration * percentage) / 100;
    }
    else if (strchr(progress_str, ':'))
    {
        int h = 0, m = 0, s = 0;
        if (sscanf(progress_str, "%d:%d:%d", &h, &m, &s) == 3)
            return h * 3600 + m * 60 + s;
        if (sscanf(progress_str, "%d:%d", &m, &s) == 2)
            return m * 60 + s;
    }
    else
    {
        for (size_t i = 0; i < strlen(progress_str); i++)
            if (!isdigit(progress_str[i]))
                return -1;
        return atoll(progress_str);
    }
    return -1;
}

void action_update_progress(int video_number, const char *progress_str)
{
    load_data_from_json();
    if (video_number <= 0 || (size_t)video_number > g_video_count)
    {
        fprintf(stderr, "Error: Invalid video number: %d. Must be between 1 and %zu.\n", video_number, g_video_count);
        return;
    }

    VideoInfo *vid = g_video_list[video_number - 1];
    long long new_watched_sec = parse_progress_string(progress_str, vid->duration_sec);

    if (new_watched_sec < 0)
    {
        fprintf(stderr, "Error: Invalid progress format: '%s'.\n", progress_str);
        return;
    }

    vid->watched_sec = new_watched_sec > vid->duration_sec ? vid->duration_sec : new_watched_sec;
    save_data_to_json();
    printf("Updated video %d ('%s') to %lld seconds.\n", video_number, vid->path, vid->watched_sec);
}

void cleanup_globals()
{
    cleanup_video_list();
    cleanup_course_root();
    free(g_course_name);
    g_course_name = NULL;
}
