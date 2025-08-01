#define _DEFAULT_SOURCE
#include "file_utils.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

int is_video_file(const char *filepath)
{
    if (!filepath)
        return 0;
        
    // Check file extension first for basic filtering
    const char *ext = strrchr(filepath, '.');
    if (ext) {
        ext++; // Move past the dot
        if (strcasecmp(ext, "mp4") == 0 || strcasecmp(ext, "avi") == 0 || 
            strcasecmp(ext, "mkv") == 0 || strcasecmp(ext, "mov") == 0 ||
            strcasecmp(ext, "wmv") == 0 || strcasecmp(ext, "flv") == 0 ||
            strcasecmp(ext, "webm") == 0 || strcasecmp(ext, "m4v") == 0) {
            return 1;
        }
    }
    
    // Fallback to mime type detection for unknown extensions
    char command[2048];
    int ret = snprintf(command, sizeof(command), "file -b --mime-type \"%s\" 2>/dev/null", filepath);
    if (ret >= (int)sizeof(command) || ret < 0) {
        return 0; // Command too long or error
    }

    FILE *pipe = popen(command, "r");
    if (!pipe)
    {
        return 0; // Don't print error, just return 0
    }

    char buffer[128];
    char *line = fgets(buffer, sizeof(buffer), pipe);
    int pclose_result = pclose(pipe);
    
    // Check if pclose succeeded and we got valid output
    if (pclose_result != 0 || !line)
    {
        return 0;
    }

    if (strncmp(line, "video/", 6) == 0)
    {
        return 1;
    }
    return 0;
}

long long get_duration_in_seconds(const char *filepath)
{
    AVFormatContext *pFormatCtx = NULL;
    if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0)
    {
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        avformat_close_input(&pFormatCtx);
        return -2;
    }

    long long duration = pFormatCtx->duration;
    avformat_close_input(&pFormatCtx);

    return (duration > 0) ? (duration / AV_TIME_BASE) : 0;
}
