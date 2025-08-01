#ifndef FILE_UTILS_H
#define FILE_UTILS_H

// Checks if a file is a video by executing the 'file' command.
int is_video_file(const char *filepath);

// Gets the duration of a video file in seconds using FFmpeg.
long long get_duration_in_seconds(const char *filepath);

#endif // FILE_UTILS_H
