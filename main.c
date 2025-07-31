#define _DEFAULT_SOURCE // Enable strdup function
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

// FFmpeg headers
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

// Jansson header for JSON manipulation
#include <jansson.h>

#define DATA_FILE ".mirava_data.json"

// A structure to hold all information about a single video file.
typedef struct
{
    char *path;
    long long duration_sec;
    long long watched_sec; // For future use
} VideoInfo;

// A dynamic array to store pointers to all found videos.
static VideoInfo **g_video_list = NULL;
static size_t g_video_count = 0;
static size_t g_video_capacity = 0;

/**
 * @brief Adds a new video to our global list.
 * Manages resizing the list if it gets full.
 */
void add_video_to_list(VideoInfo *video)
{
    if (g_video_count >= g_video_capacity)
    {
        // Double the capacity or start with 16 if it's empty
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

/**
 * @brief Frees all memory associated with the global video list.
 */
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
    }
}

/**
 * @brief Saves all the collected video information into a JSON file.
 */
void save_videos_to_json()
{
    // Create a new JSON array (the root of our file)
    json_t *root = json_array();
    if (!root)
    {
        fprintf(stderr, "Error: Could not create JSON array.\n");
        return;
    }

    // For each video in our list, create a JSON object and add it to the array
    for (size_t i = 0; i < g_video_count; i++)
    {
        VideoInfo *vid = g_video_list[i];
        json_t *video_obj = json_pack("{s:s, s:I, s:I}",
                                      "path", vid->path,
                                      "duration_sec", (json_int_t)vid->duration_sec,
                                      "watched_sec", (json_int_t)vid->watched_sec);

        if (video_obj)
        {
            json_array_append_new(root, video_obj);
        }
    }

    // Dump the JSON array to a file with pretty printing
    if (json_dump_file(root, DATA_FILE, JSON_INDENT(2)) != 0)
    {
        fprintf(stderr, "Error: Failed to write to JSON file '%s'.\n", DATA_FILE);
    }

    // Decrease the reference count for the array, which will free it
    json_decref(root);
}

long long get_duration_in_seconds(const char *filepath)
{
    AVFormatContext *pFormatCtx = NULL;
    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
        return -1;
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        avformat_close_input(&pFormatCtx);
        return -2;
    }
    long long duration = pFormatCtx->duration;
    avformat_close_input(&pFormatCtx);
    return (duration > 0) ? (duration / AV_TIME_BASE) : 0;
}

void find_videos(const char *basePath)
{
    char path[1024];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        snprintf(path, sizeof(path), "%s/%s", basePath, dp->d_name);

        // Skip our own data file
        if (strcmp(path, "./" DATA_FILE) == 0)
            continue;

        struct stat statbuf;
        if (stat(path, &statbuf) != -1)
        {
            if (S_ISDIR(statbuf.st_mode))
            {
                find_videos(path);
            }
            else
            {
                long long duration_sec = get_duration_in_seconds(path);
                if (duration_sec >= 0)
                {
                    const char *display_path = (strncmp(path, "./", 2) == 0) ? path + 2 : path;

                    VideoInfo *new_video = malloc(sizeof(VideoInfo));
                    if (new_video)
                    {
                        new_video->path = strdup(display_path);
                        new_video->duration_sec = duration_sec;
                        new_video->watched_sec = 0; // Default to 0
                        add_video_to_list(new_video);
                    }
                }
            }
        }
    }
    closedir(dir);
}

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    av_log_set_level(AV_LOG_QUIET);

    printf("Scanning for videos...\n");
    find_videos(".");

    long long total_duration_seconds = 0;
    printf("\n--- Found Videos ---\n");
    for (size_t i = 0; i < g_video_count; i++)
    {
        VideoInfo *vid = g_video_list[i];
        total_duration_seconds += vid->duration_sec;
        int h = vid->duration_sec / 3600;
        int m = (vid->duration_sec % 3600) / 60;
        int s = vid->duration_sec % 60;
        printf("%-60s [%02d:%02d:%02d]\n", vid->path, h, m, s);
    }

    if (total_duration_seconds > 0)
    {
        int h = total_duration_seconds / 3600;
        int m = (total_duration_seconds % 3600) / 60;
        int s = total_duration_seconds % 60;
        printf("------------------------------------------------------------------\n");
        printf("Total Duration: %d hours, %d minutes, and %d seconds\n", h, m, s);
    }

    printf("\nSaving data to '%s'...\n", DATA_FILE);
    save_videos_to_json();
    printf("Data saved successfully.\n");

    // Clean up all allocated memory before exiting
    cleanup_video_list();

    return 0;
}
