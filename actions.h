#ifndef ACTIONS_H
#define ACTIONS_H

// The default action: syncs filesystem, lists videos, and saves.
void action_list_and_sync();

// Updates a video's watched time and saves the result.
void action_update_progress(int video_number, const char* progress_str);

// Frees all global resources.
void cleanup_globals();

#endif // ACTIONS_H
