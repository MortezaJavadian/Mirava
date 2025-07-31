#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>

// FFmpeg headers
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

// Structure to hold a file signature (magic number)
typedef struct
{
    const unsigned char *signature;
    size_t signature_len;
    size_t offset;
} FileSignature;

/**
 * @brief Gets the duration of a video file using FFmpeg.
 *
 * @param filepath The path to the video file.
 * @return char* A dynamically allocated string in [HH:MM:SS] format.
 * The caller is responsible for freeing this memory.
 * Returns "[Unknown]" or "[No Duration]" on failure.
 */
char *get_video_duration(const char *filepath)
{
    AVFormatContext *pFormatCtx = NULL;
    // Allocate memory for the duration string. e.g., "[01:23:45]"
    char *duration_str = malloc(12);
    if (!duration_str)
    {
        return NULL; // Failed to allocate memory
    }

    // Open video file
    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
    {
        snprintf(duration_str, 12, "[Unknown]");
        return duration_str; // Couldn't open file
    }

    // Retrieve stream information
    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        snprintf(duration_str, 12, "[Unknown]");
        avformat_close_input(&pFormatCtx);
        return duration_str; // Couldn't find stream information
    }

    // Duration is in AV_TIME_BASE units (microseconds)
    long long duration = pFormatCtx->duration;

    if (duration > 0)
    {
        long long total_seconds = duration / AV_TIME_BASE;
        int hours = total_seconds / 3600;
        int minutes = (total_seconds % 3600) / 60;
        int seconds = total_seconds % 60;
        snprintf(duration_str, 12, "[%02d:%02d:%02d]", hours, minutes, seconds);
    }
    else
    {
        snprintf(duration_str, 12, "[No Duration]");
    }

    // Close the video file and free the context
    avformat_close_input(&pFormatCtx);

    return duration_str;
}

int is_video_by_signature(const char *filepath)
{
    FileSignature video_signatures[] = {
        {(const unsigned char *)"\x1A\x45\xDF\xA3", 4, 0},                 // MKV
        {(const unsigned char *)"AVI ", 4, 8},                             // AVI
        {(const unsigned char *)"ftyp", 4, 4},                             // MP4, MOV
        {(const unsigned char *)"FLV", 3, 0},                              // FLV
        {(const unsigned char *)"\x30\x26\xB2\x75\x8E\x66\xCF\x11", 8, 0}, // WMV
        {(const unsigned char *)"\x00\x00\x01\xBA", 4, 0},                 // MPEG-PS
        {(const unsigned char *)"\x47", 1, 0}                              // MPEG-TS
    };
    int num_signatures = sizeof(video_signatures) / sizeof(video_signatures[0]);

    FILE *file = fopen(filepath, "rb");
    if (!file)
        return 0;

    unsigned char buffer[32];
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    for (int i = 0; i < num_signatures; i++)
    {
        FileSignature sig = video_signatures[i];
        if (bytes_read >= sig.offset + sig.signature_len)
        {
            if (memcmp(buffer + sig.offset, sig.signature, sig.signature_len) == 0)
            {
                if (sig.offset == 8 && strncmp((const char *)buffer, "RIFF", 4) != 0)
                    continue;
                return 1;
            }
        }
    }
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
                        char *duration = get_video_duration(path);
                        const char *display_path = (strncmp(path, "./", 2) == 0) ? path + 2 : path;

                        // Print filename and duration, aligned for readability
                        printf("%-60s %s\n", display_path, duration);

                        // Free the memory allocated by get_video_duration
                        free(duration);
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

    printf("Searching for video files...\n\n");
    find_videos(".");
    printf("\nSearch complete.\n");

    return 0;
}
