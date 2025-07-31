#include <stdio.h>
#include <string.h>
#include <dirent.h>   // For directory operations
#include <sys/stat.h> // For checking file/directory status
#include <stdlib.h>   // For malloc and free

// Structure to hold a file signature (magic number)
typedef struct
{
    const unsigned char *signature; // The byte sequence to look for
    size_t signature_len;           // How long the sequence is
    size_t offset;                  // Where to start looking in the file
} FileSignature;

/**
 * @brief Checks if a file is a video by reading its first few bytes (magic numbers).
 *
 * @param filepath The path to the file to check.
 * @return int Returns 1 if a known video signature is found, 0 otherwise.
 */
int is_video_by_signature(const char *filepath)
{
    // A list of known video file signatures.
    FileSignature video_signatures[] = {
        // Matroska (MKV) & WebM: Starts with 0x1A 0x45 0xDF 0xA3
        {(const unsigned char *)"\x1A\x45\xDF\xA3", 4, 0},
        // AVI: Starts with "RIFF", has "AVI " at offset 8
        {(const unsigned char *)"AVI ", 4, 8},
        // MP4, MOV: Has "ftyp" at offset 4
        {(const unsigned char *)"ftyp", 4, 4},
        // FLV: Starts with "FLV"
        {(const unsigned char *)"FLV", 3, 0},
        // WMV/ASF: Starts with a specific GUID
        {(const unsigned char *)"\x30\x26\xB2\x75\x8E\x66\xCF\x11", 8, 0},
        // MPEG-PS: Starts with 0x00 0x00 0x01 0xBA
        {(const unsigned char *)"\x00\x00\x01\xBA", 4, 0},
        // MPEG-TS: Starts with 0x47 (sync byte)
        {(const unsigned char *)"\x47", 1, 0}};
    int num_signatures = sizeof(video_signatures) / sizeof(video_signatures[0]);

    FILE *file = fopen(filepath, "rb"); // Open in binary read mode
    if (!file)
    {
        // Cannot open file, assume it's not a video we can process
        return 0;
    }

    // Read a small chunk from the beginning of the file
    unsigned char buffer[32]; // 32 bytes is enough for most checks
    size_t bytes_read = fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    // Now, check the buffer against our known signatures
    for (int i = 0; i < num_signatures; i++)
    {
        FileSignature sig = video_signatures[i];
        // Ensure we have read enough bytes to perform the check
        if (bytes_read >= sig.offset + sig.signature_len)
        {
            // Compare the bytes from the file with the known signature
            if (memcmp(buffer + sig.offset, sig.signature, sig.signature_len) == 0)
            {
                // For AVI, we also need to check for "RIFF" at the beginning
                if (sig.offset == 8 && strncmp((const char *)buffer, "RIFF", 4) != 0)
                {
                    continue; // Not a valid AVI file
                }
                return 1; // Match found!
            }
        }
    }

    return 0; // No known video signature found
}

/**
 * @brief Recursively finds and prints video files in a directory.
 *
 * @param basePath The path of the directory to start searching from.
 */
void find_videos(const char *basePath)
{
    char path[1024];
    struct dirent *dp;
    DIR *dir = opendir(basePath);

    if (!dir)
    {
        return;
    }

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
        {
            snprintf(path, sizeof(path), "%s/%s", basePath, dp->d_name);

            struct stat statbuf;
            if (stat(path, &statbuf) != -1)
            {
                if (S_ISDIR(statbuf.st_mode))
                {
                    find_videos(path);
                }
                else
                {
                    // Use the new signature checking function
                    if (is_video_by_signature(path))
                    {
                        printf("%s\n", path);
                    }
                }
            }
        }
    }

    closedir(dir);
}

/**
 * @brief The main entry point for the mirava application.
 */
int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    printf("Searching for video files by signature...\n");
    find_videos(".");
    printf("Search complete.\n");

    return 0;
}
