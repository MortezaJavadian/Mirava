#include "actions.h"
#include "cli.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libavformat/avformat.h>

int main(int argc, char *argv[])
{
    // Suppress FFmpeg logs for cleaner output
    av_log_set_level(AV_LOG_QUIET);

    if (argc == 1)
    {
        action_list_and_sync();
    }
    else if (strcmp(argv[1], "help") == 0)
    {
        show_help();
    }
    else if (strcmp(argv[1], "set") == 0)
    {
        if (argc != 4)
        {
            fprintf(stderr, "Error: 'set' command requires a video number and a progress value.\n");
            show_help();
            return 1;
        }
        action_update_progress(atoi(argv[2]), argv[3]);
    }
    else if (strcmp(argv[1], "mark") == 0)
    {
        if (argc < 3)
        {
            fprintf(stderr, "Error: 'mark' command requires at least one video number.\n");
            show_help();
            return 1;
        }
        
        // Mark multiple videos
        for (int i = 2; i < argc; i++)
        {
            int video_num = atoi(argv[i]);
            if (video_num <= 0)
            {
                fprintf(stderr, "Error: Invalid video number '%s'. Must be a positive integer.\n", argv[i]);
                return 1;
            }
            action_update_progress(video_num, "100%");
        }
    }
    else
    {
        fprintf(stderr, "Error: Unknown command '%s'.\n", argv[1]);
        show_help();
        return 1;
    }

    cleanup_globals();
    return 0;
}
