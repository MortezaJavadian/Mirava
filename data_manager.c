#define _DEFAULT_SOURCE
#include "data_manager.h"
#include "video_list.h"
#include "globals.h" // Use the centralized global declarations
#include <jansson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <libgen.h>

#define DATA_FILE ".mirava_data.json"

// Global variable to store the course root directory
static char *g_course_root_dir = NULL;

// Function to find the course root directory by searching for .mirava_data.json
// Returns the path to the directory containing the JSON file, or NULL if not found
static char* find_course_root()
{
    char current_dir[PATH_MAX];
    char json_path[PATH_MAX];
    char search_path[PATH_MAX];
    struct stat st;
    
    // Get current working directory
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        return NULL;
    }
    
    strcpy(search_path, current_dir);
    
    while (1) {
        // Build path to potential JSON file
        snprintf(json_path, sizeof(json_path), "%s/%s", search_path, DATA_FILE);
        
        // Check if JSON file exists
        if (stat(json_path, &st) == 0 && S_ISREG(st.st_mode)) {
            // Found it! Return the directory path
            return strdup(search_path);
        }
        
        // Check if we're at root directory
        if (strcmp(search_path, "/") == 0) {
            break;
        }
        
        // Find the last slash to move to parent directory
        char *last_slash = strrchr(search_path, '/');
        if (last_slash == NULL || last_slash == search_path) {
            // Can't go up anymore
            break;
        }
        
        // Truncate at the last slash to get parent directory
        *last_slash = '\0';
        
        // Handle root directory case
        if (strlen(search_path) == 0) {
            strcpy(search_path, "/");
        }
    }
    
    return NULL;
}

void load_data_from_json()
{
    json_t *root;
    json_error_t error;
    char json_path[PATH_MAX];
    
    // First, try to find course root directory
    free(g_course_root_dir);
    g_course_root_dir = find_course_root();
    
    if (g_course_root_dir) {
        // Found existing course, load from its directory
        snprintf(json_path, sizeof(json_path), "%s/%s", g_course_root_dir, DATA_FILE);
        
        // Change to course root directory to scan videos from there
        char original_dir[PATH_MAX];
        getcwd(original_dir, sizeof(original_dir));
        
        if (chdir(g_course_root_dir) == 0) {
            root = json_load_file(DATA_FILE, 0, &error);
            // Change back to original directory
            chdir(original_dir);
        } else {
            root = NULL;
        }
    } else {
        // No existing course found, treat current directory as new course
        root = json_load_file(DATA_FILE, 0, &error);
        
        // Set current directory as course root
        char current_dir[PATH_MAX];
        if (getcwd(current_dir, sizeof(current_dir)) != NULL) {
            g_course_root_dir = strdup(current_dir);
        }
    }
    
    if (!root)
        return;

    json_t *name_obj = json_object_get(root, "course_name");
    if (json_is_string(name_obj))
    {
        free(g_course_name);
        g_course_name = strdup(json_string_value(name_obj));
    }

    json_t *videos_array = json_object_get(root, "videos");
    if (json_is_array(videos_array))
    {
        size_t index;
        json_t *value;
        json_array_foreach(videos_array, index, value)
        {
            const char *path = json_string_value(json_object_get(value, "path"));
            json_int_t duration = json_integer_value(json_object_get(value, "duration_sec"));
            json_int_t watched = json_integer_value(json_object_get(value, "watched_sec"));

            if (path)
            {
                VideoInfo *vid = malloc(sizeof(VideoInfo));
                if (vid)
                {
                    vid->path = strdup(path);
                    vid->duration_sec = duration;
                    vid->watched_sec = watched;
                    vid->found_on_disk = 0;
                    add_video_to_list(vid);
                }
            }
        }
    }
    json_decref(root);
}

void save_data_to_json()
{
    json_t *root = json_object();
    json_object_set_new(root, "course_name", json_string(g_course_name ? g_course_name : ""));

    json_t *videos_array = json_array();
    for (size_t i = 0; i < g_video_count; i++)
    {
        VideoInfo *vid = g_video_list[i];
        json_t *video_obj = json_pack("{s:s, s:I, s:I}",
                                      "path", vid->path,
                                      "duration_sec", (json_int_t)vid->duration_sec,
                                      "watched_sec", (json_int_t)vid->watched_sec);
        if (video_obj)
        {
            json_array_append_new(videos_array, video_obj);
        }
    }
    json_object_set_new(root, "videos", videos_array);

    char json_path[PATH_MAX];
    if (g_course_root_dir) {
        snprintf(json_path, sizeof(json_path), "%s/%s", g_course_root_dir, DATA_FILE);
    } else {
        snprintf(json_path, sizeof(json_path), "%s", DATA_FILE);
    }

    if (json_dump_file(root, json_path, JSON_INDENT(2)) != 0)
    {
        fprintf(stderr, "Error: Failed to write to JSON file '%s'.\n", json_path);
    }
    json_decref(root);
}

// Function to get the course root directory
const char* get_course_root_dir()
{
    return g_course_root_dir;
}

// Function to cleanup course root directory
void cleanup_course_root()
{
    free(g_course_root_dir);
    g_course_root_dir = NULL;
}
