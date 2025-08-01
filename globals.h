#ifndef GLOBALS_H
#define GLOBALS_H

#include "types.h"
#include <stddef.h>

// Declare global variables that will be defined in one .c file (video_list.c)
// Using 'extern' tells the compiler these exist but are defined elsewhere.
extern char *g_course_name;
extern VideoInfo **g_video_list;
extern size_t g_video_count;
extern size_t g_video_capacity;

#endif // GLOBALS_H
