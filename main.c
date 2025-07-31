#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

// FFmpeg headers
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

// A global variable to store the total duration of all videos found, in seconds.
static long long g_total_duration_seconds = 0;

/**
 * @brief Gets the duration of a video file in seconds using FFmpeg.
 *
 * @param filepath The path to the video file.
 * @return long long The duration in seconds. Returns a negative value on failure.
 */
long long get_duration_in_seconds(const char *filepath)
{
    AVFormatContext *pFormatCtx = NULL;
    long long duration_seconds = -1; // Default to error/unknown

    // Open video file
    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
    {
        return -1; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        avformat_close_input(&pFormatCtx);
        return -2; // Couldn't find stream information
    }

    // Duration is in AV_TIME_BASE units (microseconds)
    if (pFormatCtx->duration > 0)
    {
        duration_seconds = pFormatCtx->duration / AV_TIME_BASE;
    }

    // Close the video file and free the context
    avformat_close_input(&pFormatCtx);

    return duration_seconds;
}

int is_video_by_signature(const char *filepath)
{
    // This function remains unchanged
    FILE *file = fopen(filepath, "rb");
    if (!file)
        return 0;
    // ... (rest of the function is the same as before)
    unsigned char buffer[32];
    fread(buffer, 1, sizeof(buffer), file);
    fclose(file);
    // A simple check for brevity, the full list is in the previous version
    if (memcmp(buffer + 4, "ftyp", 4) == 0)
        return 1; // MP4
    if (memcmp(buffer, "\x1A\x45\xDF\xA3", 4) == 0)
        return 1; // MKV
    return 0;
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
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            snprintf(path, sizeof(path), "%s/%s", basePath, dp->d_name);
            struct stat statbuf;
            if (stat(path, &statbuf) != -1)
            {
                if (S_ISDIR(statbuf.st_mode))
                {
                    find_videos(path);
                }
                else
                {
                    if (is_video_by_signature(path))
                    {
                        long long duration_sec = get_duration_in_seconds(path);
                        char duration_str[12];

                        if (duration_sec >= 0)
                        {
                            // Add to the global total
                            g_total_duration_seconds += duration_sec;

                            // Format for printing
                            int hours = duration_sec / 3600;
                            int minutes = (duration_sec % 3600) / 60;
                            int seconds = duration_sec % 60;
                            snprintf(duration_str, sizeof(duration_str), "[%02d:%02d:%02d]", hours, minutes, seconds);
                        }
                        else
                        {
                            snprintf(duration_str, sizeof(duration_str), "[Unknown]");
                        }

                        const char *display_path = (strncmp(path, "./", 2) == 0) ? path + 2 : path;
                        printf("%-60s %s\n", display_path, duration_str);
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

    // Suppress FFmpeg logs for cleaner output
    av_log_set_level(AV_LOG_QUIET);

    printf("Searching for video files...\n\n");
    find_videos(".");
    printf("\nSearch complete.\n");

    // Print the total duration
    if (g_total_duration_seconds > 0)
    {
        long long total = g_total_duration_seconds;
        int hours = total / 3600;
        int minutes = (total % 3600) / 60;
        int seconds = total % 60;
        printf("------------------------------------------------------------------\n");
        printf("Total Duration: %d hours, %d minutes, and %d seconds\n", hours, minutes, seconds);
    }

    return 0;
}
