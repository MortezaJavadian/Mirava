#define _DEFAULT_SOURCE // Enable strdup, basename
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h> // For getcwd, access
#include <libgen.h> // For basename

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
    long long watched_sec;
    int found_on_disk; // A flag to sync with filesystem
} VideoInfo;

// --- Global Variables ---
static VideoInfo **g_video_list = NULL;
static size_t g_video_count = 0;
static size_t g_video_capacity = 0;
static char *g_course_name = NULL;

// Forward declarations
void add_video_to_list(VideoInfo *video);
long long get_duration_in_seconds(const char *filepath);

/**
 * @brief Loads all data (course name and video list) from the JSON file.
 * Populates the global variables.
 */
void load_data_from_json()
{
    json_t *root;
    json_error_t error;

    root = json_load_file(DATA_FILE, 0, &error);
    if (!root)
    {
        // This is not an error if the file doesn't exist on first run.
        return;
    }

    // Load course name
    json_t *name_obj = json_object_get(root, "course_name");
    if (json_is_string(name_obj))
    {
        g_course_name = strdup(json_string_value(name_obj));
    }

    // Load videos array
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
                vid->path = strdup(path);
                vid->duration_sec = duration;
                vid->watched_sec = watched;
                vid->found_on_disk = 0; // Mark as not found yet
                add_video_to_list(vid);
            }
        }
    }
    json_decref(root);
}

/**
 * @brief Finds a video in the global list by its path.
 * @return Pointer to the VideoInfo struct or NULL if not found.
 */
VideoInfo *find_video_by_path(const char *path)
{
    for (size_t i = 0; i < g_video_count; i++)
    {
        if (strcmp(g_video_list[i]->path, path) == 0)
        {
            return g_video_list[i];
        }
    }
    return NULL;
}

/**
 * @brief Scans the filesystem for videos and syncs with the loaded data.
 */
void scan_and_sync_videos(const char *basePath)
{
    char full_path[1024];
    struct dirent *dp;
    DIR *dir = opendir(basePath);
    if (!dir)
        return;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        snprintf(full_path, sizeof(full_path), "%s/%s", basePath, dp->d_name);
        const char *display_path = (strncmp(full_path, "./", 2) == 0) ? full_path + 2 : full_path;

        if (strcmp(display_path, DATA_FILE) == 0)
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
                    // Video exists, mark it as found and update duration
                    existing_video->found_on_disk = 1;
                    existing_video->duration_sec = get_duration_in_seconds(full_path);
                }
                else
                {
                    // It's a new video
                    VideoInfo *new_video = malloc(sizeof(VideoInfo));
                    new_video->path = strdup(display_path);
                    new_video->duration_sec = get_duration_in_seconds(full_path);
                    new_video->watched_sec = 0;
                    new_video->found_on_disk = 1;
                    add_video_to_list(new_video);
                }
            }
        }
    }
    closedir(dir);
}

/**
 * @brief Removes videos from the list that were not found on disk.
 */
void prune_missing_videos()
{
    VideoInfo **new_list = malloc(g_video_capacity * sizeof(VideoInfo *));
    size_t new_count = 0;

    for (size_t i = 0; i < g_video_count; i++)
    {
        if (g_video_list[i]->found_on_disk)
        {
            new_list[new_count++] = g_video_list[i];
        }
        else
        {
            // This video was in the JSON but not on disk, free it.
            free(g_video_list[i]->path);
            free(g_video_list[i]);
        }
    }

    free(g_video_list);
    g_video_list = new_list;
    g_video_count = new_count;
}

// --- Utility functions (unchanged or minor changes) ---
int is_video_file(const char *filepath)
{
    char command[2048];
    snprintf(command, sizeof(command), "file -b --mime-type \"%s\"", filepath);
    FILE *pipe = popen(command, "r");
    if (!pipe)
        return 0;
    char buffer[128];
    char *line = fgets(buffer, sizeof(buffer), pipe);
    pclose(pipe);
    return (line && strncmp(line, "video/", 6) == 0);
}

void prompt_for_course_name()
{
    char input_buffer[256];
    printf("Enter the course name (leave blank for current directory name): ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin))
    {
        input_buffer[strcspn(input_buffer, "\n")] = 0;
        if (strlen(input_buffer) == 0)
        {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL)
            {
                char *cwd_copy = strdup(cwd);
                g_course_name = strdup(basename(cwd_copy));
                free(cwd_copy);
            }
            else
            {
                g_course_name = strdup("Untitled");
            }
        }
        else
        {
            g_course_name = strdup(input_buffer);
        }
    }
}

void save_videos_to_json()
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
            json_array_append_new(videos_array, video_obj);
    }
    json_object_set_new(root, "videos", videos_array);
    json_dump_file(root, DATA_FILE, JSON_INDENT(2));
    json_decref(root);
}

void add_video_to_list(VideoInfo *video)
{
    if (g_video_count >= g_video_capacity)
    {
        g_video_capacity = g_video_capacity == 0 ? 16 : g_video_capacity * 2;
        g_video_list = realloc(g_video_list, g_video_capacity * sizeof(VideoInfo *));
    }
    g_video_list[g_video_count++] = video;
}

void cleanup()
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
    free(g_course_name);
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

int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    av_log_set_level(AV_LOG_QUIET);

    // 1. Load existing data from .json file
    load_data_from_json();

    // 2. If no course name is loaded, prompt the user for one
    if (!g_course_name)
    {
        prompt_for_course_name();
    }

    // 3. Scan filesystem and sync: find new files, mark existing ones
    scan_and_sync_videos(".");

    // 4. Prune: remove videos from list that are no longer on disk
    prune_missing_videos();

    // 5. Display the final, synced list
    printf("\n--- Course: %s ---\n", g_course_name);
    long long total_duration_seconds = 0;
    for (size_t i = 0; i < g_video_count; i++)
    {
        VideoInfo *vid = g_video_list[i];
        total_duration_seconds += vid->duration_sec;

        char status_str[15] = "";
        if (vid->duration_sec > 0)
        {
            if (vid->watched_sec >= vid->duration_sec)
            {
                snprintf(status_str, sizeof(status_str), "[✓]");
            }
            else if (vid->watched_sec > 0)
            {
                int percentage = (int)(100 * vid->watched_sec / vid->duration_sec);
                snprintf(status_str, sizeof(status_str), "[%d%%]", percentage);
            }
        }

        int h = vid->duration_sec / 3600;
        int m = (vid->duration_sec % 3600) / 60;
        int s = vid->duration_sec % 60;
        printf("%-50s [%02d:%02d:%02d] %s\n", vid->path, h, m, s, status_str);
    }

    if (total_duration_seconds > 0)
    {
        int h = total_duration_seconds / 3600;
        int m = (total_duration_seconds % 3600) / 60;
        int s = total_duration_seconds % 60;
        printf("-----------------------------------------------------------------------------\n");
        printf("Total Duration: %d hours, %d minutes, and %d seconds\n", h, m, s);
    }

    // 6. Save the updated data back to the .json file
    save_videos_to_json();
    printf("\nData synced and saved successfully.\n");

    cleanup();
    return 0;
}
