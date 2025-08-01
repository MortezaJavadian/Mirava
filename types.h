#ifndef TYPES_H
#define TYPES_H

// A structure to hold all information about a single video file.
typedef struct {
    char *path;
    long long duration_sec;
    long long watched_sec;
    int found_on_disk; // A flag to sync with filesystem
} VideoInfo;

#endif // TYPES_H
