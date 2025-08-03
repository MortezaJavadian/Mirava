#define _DEFAULT_SOURCE
#include "cli.h"
#include "globals.h" // Use the centralized global declarations
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

void display_video_list()
{
    printf("\n--- Course: %s ---\n", g_course_name ? g_course_name : "N/A");
    long long total_duration = 0;
    long long total_watched = 0;

    for (size_t i = 0; i < g_video_count; i++)
    {
        VideoInfo *vid = g_video_list[i];
        total_duration += vid->duration_sec;
        total_watched += vid->watched_sec;

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
        else
        {
            // For videos with unknown duration, check if they are marked as watched
            if (vid->watched_sec >= 999999) // Our arbitrary "complete" value
            {
                snprintf(status_str, sizeof(status_str), "[✓]");
            }
            else if (vid->watched_sec > 0)
            {
                snprintf(status_str, sizeof(status_str), "[watched]");
            }
        }

        int h = vid->duration_sec / 3600;
        int m = (vid->duration_sec % 3600) / 60;
        int s = vid->duration_sec % 60;
        printf("%2zu. %-50s [%02d:%02d:%02d] %s\n", i + 1, vid->path, h, m, s, status_str);
    }

    printf("---------------------------------------------------------------------\n");
    if (total_duration > 0)
    {
        int h = total_duration / 3600;
        int m = (total_duration % 3600) / 60;
        int s = total_duration % 60;
        int overall_percent = (int)(100 * total_watched / total_duration);
        printf("Total Duration: %d:%02d:%02d  |  Overall Progress: %d%%\n", h, m, s, overall_percent);
    }
}

void prompt_for_course_name()
{
    char input_buffer[256];
    printf("Enter the course name (leave blank for current directory name): ");
    if (fgets(input_buffer, sizeof(input_buffer), stdin))
    {
        input_buffer[strcspn(input_buffer, "\n")] = 0;

        free(g_course_name);
        g_course_name = NULL;

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
        }
        else
        {
            g_course_name = strdup(input_buffer);
        }
    }
}

void show_help()
{
    printf("mirava - A simple video course progress tracker.\n\n");
    printf("Usage:\n");
    printf("  mirava                     - List videos and sync progress.\n");
    printf("  mirava set <num> <val>     - Set progress for video <num>.\n");
    printf("  mirava mark <num> [num...] - Mark video(s) as complete.\n");
    printf("  mirava help                - Show this help message.\n\n");
    printf("Examples:\n");
    printf("  mirava set 3 50%%            - Set video 3 to 50%% watched.\n");
    printf("  mirava set 5 1:20:10         - Set video 5 to 1h 20m 10s watched.\n");
    printf("  mirava mark 8              - Mark video 8 as 100%% watched.\n");
    printf("  mirava mark 3 5 7          - Mark videos 3, 5, and 7 as 100%% watched.\n");
}
