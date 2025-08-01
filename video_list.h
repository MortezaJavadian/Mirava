#ifndef VIDEO_LIST_H
#define VIDEO_LIST_H

#include "types.h"
#include <stddef.h>

// Adds a new video to the global list.
void add_video_to_list(VideoInfo *video);

// Finds a video in the global list by its path.
VideoInfo* find_video_by_path(const char *path);

// Removes videos from the list that were not found on disk.
void prune_missing_videos();

// Frees all memory associated with the global video list.
void cleanup_video_list();

#endif // VIDEO_LIST_H
