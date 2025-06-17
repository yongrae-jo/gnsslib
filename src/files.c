// =============================================================================
// Files implementation
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#include "files.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// Static functions (internal use only)
// =============================================================================

// Initialize file string structure (internal)
static void InitFileStr(files_t *files)
{
    if (!files) return;

    files->names = NULL;
    files->n = 0;
    files->nmax = 0;

    // Initial capacity is 2
    files->names = (char**)malloc(2 * sizeof(char*));
    if (files->names) {
        files->nmax = 2;
        // Initialize all pointers to NULL
        for (int i = 0; i < 2; i++) {
            files->names[i] = NULL;
        }
    }
}

// Free file string structure (internal)
static void FreeFileStr(files_t *files)
{
    if (!files) return;

    if (files->names) {
        // Free each individual file name
        for (int i = 0; i < files->n; i++) {
            if (files->names[i]) {
                free(files->names[i]);
                files->names[i] = NULL;
            }
        }
        // Free the array of pointers
        free(files->names);
        files->names = NULL;
    }

    files->n = 0;
    files->nmax = 0;
}

// Resize file string structure (internal function)
static int ResizeFileStr(files_t *files, int nnew)
{
    if (!files || nnew <= 0) return 0;

    char **new_names = (char**)realloc(files->names, nnew * sizeof(char*));
    if (!new_names) return 0;

    files->names = new_names;

    // Initialize new pointers to NULL
    for (int i = files->nmax; i < nnew; i++) {
        files->names[i] = NULL;
    }

    files->nmax = nnew;
    return 1;
}

// =============================================================================
// File string functions
// =============================================================================

// Add file name to file string structure
int AddFileName(files_t *files, const char *filename)
{
    if (!files || !filename) return 0;

    // Check if we need to resize
    if (files->n >= files->nmax) {
        int nnew = (files->nmax == 0) ? 2 : files->nmax * 2;
        if (!ResizeFileStr(files, nnew)) return 0;
    }

    // Allocate memory for the new file name
    int len = strlen(filename);
    if (len >= MAX_FILE_NAME_LEN) return 0;

    files->names[files->n] = (char*)malloc((len + 1) * sizeof(char));
    if (!files->names[files->n]) return 0;

    // Copy the file name
    strcpy(files->names[files->n], filename);
    files->n++;

    return 1;
}

// Get file name from file string structure
const char *GetFileName(const files_t *files, int index)
{
    if (!files || index < 0 || index >= files->n) return NULL;
    return files->names[index];
}

// =============================================================================
// File structure functions
// =============================================================================

// Initialize file structure
void InitFile(file_t *file)
{
    if (!file) return;

    InitFileStr(&file->obsfiles);
    InitFileStr(&file->navfiles);
    InitFileStr(&file->sp3files);
    InitFileStr(&file->clkfiles);
    InitFileStr(&file->dcbfiles);
    InitFileStr(&file->atxfiles);
}

// Free file structure
void FreeFile(file_t *file)
{
    if (!file) return;

    FreeFileStr(&file->obsfiles);
    FreeFileStr(&file->navfiles);
    FreeFileStr(&file->sp3files);
    FreeFileStr(&file->clkfiles);
    FreeFileStr(&file->dcbfiles);
    FreeFileStr(&file->atxfiles);
}

// =============================================================================
// File read functions
// =============================================================================

// Read observation data files
void ReadObsFiles(files_t *files, nav_t *nav, obss_t *obs)
{
    // TODO: Implement observation file reading
}

// Read navigation data files
void ReadNavFiles(files_t *files, nav_t *nav)
{
    // TODO: Implement navigation file reading
}

// Read DCB data files
void ReadDcbFiles(files_t *files, nav_t *nav)
{
    // TODO: Implement DCB file reading
}

// Read antenna exchange data files
void ReadAtxFiles(files_t *files, nav_t *nav)
{
    // TODO: Implement antenna exchange file reading
}

// Read all files
void ReadFiles(file_t *file, nav_t *nav, obss_t *obs)
{
    if (!file || !nav || !obs) return;

    ReadObsFiles(&file->obsfiles, nav, obs);
    ReadNavFiles(&file->navfiles, nav);
    ReadDcbFiles(&file->dcbfiles, nav);
    ReadAtxFiles(&file->atxfiles, nav);
}

// =============================================================================
// End of file
// =============================================================================
