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
} VideoInfo;

// --- Global Variables ---
static VideoInfo **g_video_list = NULL;
static size_t g_video_count = 0;
static size_t g_video_capacity = 0;
static char *g_course_name = NULL;

/**
 * @brief Checks if a file is a video by executing the 'file' command.
 * This is a much more reliable method than manual signature checking.
 *
 * @param filepath The path to the file to check.
 * @return int Returns 1 if it's a video, 0 otherwise.
 */
int is_video_file(const char *filepath)
{
    char command[2048];
    // Use "file -b --mime-type" to get only the MIME type string.
    // The quotes around "%s" are crucial to handle filepaths with spaces.
    snprintf(command, sizeof(command), "file -b --mime-type \"%s\"", filepath);

    FILE *pipe = popen(command, "r");
    if (!pipe)
    {
        fprintf(stderr, "Error: popen() failed while checking file type.\n");
        return 0;
    }

    char buffer[128];
    char *line = fgets(buffer, sizeof(buffer), pipe);
    pclose(pipe);

    if (line)
    {
        // Check if the output from the 'file' command starts with "video/"
        if (strncmp(line, "video/", 6) == 0)
        {
            return 1; // It's a video file
        }
    }
    return 0; // Not a video file
}

// --- Functions for course name, JSON, and list management (mostly unchanged) ---

void prompt_for_course_name()
{
    char input_buffer[256];
    printf("Enter the course name (leave blank to use current directory name): ");
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
                g_course_name = strdup("Untitled Course");
            }
            printf("Using default course name: '%s'\n", g_course_name);
        }
        else
        {
            g_course_name = strdup(input_buffer);
        }
    }
}

char *load_course_name(const char *filename)
{
    json_t *root;
    json_error_t error;
    root = json_load_file(filename, 0, &error);
    if (!root)
        return NULL;

    json_t *name_obj = json_object_get(root, "course_name");
    if (!json_is_string(name_obj))
    {
        json_decref(root);
        return NULL;
    }
    char *course_name = strdup(json_string_value(name_obj));
    json_decref(root);
    return course_name;
}

void save_videos_to_json()
{
    json_t *root = json_object();
    if (g_course_name)
    {
        json_object_set_new(root, "course_name", json_string(g_course_name));
    }
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
        size_t new_capacity = g_video_capacity == 0 ? 16 : g_video_capacity * 2;
        VideoInfo **new_list = realloc(g_video_list, new_capacity * sizeof(VideoInfo *));
        if (!new_list)
        {
            fprintf(stderr, "Error: Failed to allocate memory.\n");
            return;
        }
        g_video_list = new_list;
        g_video_capacity = new_capacity;
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
                // Use our new, more reliable function to check the file type
                if (is_video_file(path))
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
                            new_video->watched_sec = 0; // Reset on every run for now
                            add_video_to_list(new_video);
                        }
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

    if (access(DATA_FILE, F_OK) != -1)
    {
        printf("Data file '%s' found. Loading existing data...\n", DATA_FILE);
        g_course_name = load_course_name(DATA_FILE);
        if (!g_course_name)
        {
            prompt_for_course_name();
        }
    }
    else
    {
        printf("No data file found. Starting a new course.\n");
        prompt_for_course_name();
    }

    printf("\nScanning for videos...\n");
    find_videos(".");

    long long total_duration_seconds = 0;
    printf("\n--- Course: %s ---\n", g_course_name ? g_course_name : "N/A");
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

    cleanup();
    return 0;
}
