#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

// Loads all data (course name and video list) from the JSON file.
void load_data_from_json();

// Saves all the collected video information into a JSON file.
void save_data_to_json();

// Gets the course root directory path
const char* get_course_root_dir();

// Cleanup course root directory
void cleanup_course_root();

#endif // DATA_MANAGER_H
