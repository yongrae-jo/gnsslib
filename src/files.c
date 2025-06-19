// =============================================================================
// Files implementation
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

// Standard library
#include <stdlib.h>                     // for malloc, free, realloc
#include <string.h>                     // for strlen, strcpy
#include <stdio.h>                      // for fopen, fseek, ftell, fread, fclose

// GNSS library
#include "files.h"
#include "rinex.h"                      // for IsRinexObs, ReadRnxObs
#include "obs.h"                        // for SortObss

// =============================================================================
// Macros
// =============================================================================

#define MAX_FILE_NAME_LEN 1024          // Maximum number of characters in file name

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

// Initialize file buffer (1: success, 0: failure)
int InitBuff(buffer_t *buffer)
{
    // Check if the buffer pointer is valid
    if (!buffer) return 0;

    // Initialize the buffer
    buffer->buff = NULL;
    buffer->lineinfo = NULL;
    buffer->nline = 0;

    return 1;
}

// Get file buffer (1: success, 0: failure)
int GetBuff(const char *filename, buffer_t *buffer)
{
    // Check if the buffer is valid
    if (!buffer) return 0;

    // Open the file
    FILE *fp = fopen(filename, "rb");
    if (!fp) return 0;

    // Get file size
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);

    // Allocate memory for the buffer
    char *buff = (char*)malloc(size + 1);
    if (!buff) {
        fclose(fp);
        return 0;
    }

    // Read the file into the buffer
    size_t nread = fread(buff, 1, size, fp);
    if (nread != size) {
        free(buff);
        fclose(fp);
        return 0;
    }

    // Close the file
    fclose(fp);

    // Null terminate the buffer
    buff[size] = '\0';

    // Remove '\r' from the buffer
    size_t j = 0;
    for (size_t i = 0; i < size; i++) {
        if (buff[i] != '\r') {
            buff[j++] = buff[i];
        }
    }
    buff[j] = '\0';
    size = j;

    // Count the number of lines
    size_t nline = 0;
    for (size_t i = 0; i < size; i++) {
        if (buff[i] == '\n') nline++;
    }

    // Check for last line without trailing '\n'
    if (size == 0 || buff[size-1] != '\n') nline++;

    // Allocate memory for the line information
    lineinfo_t *lineinfo = (lineinfo_t*)malloc(nline * sizeof(lineinfo_t));
    if (!lineinfo) {
        free(buff);
        return 0;
    }

    // Set the line information
    size_t start = 0;
    size_t idx = 0;
    for (size_t i = 0; i < size; i++) {
        if (buff[i] == '\n') {
            lineinfo[idx].start = start;
            lineinfo[idx].end = i - 1;
            lineinfo[idx].len = i - start;
            start = i + 1;
            idx++;
        }
    }

    // Last line if missing '\n'
    if (start < size) {
        lineinfo[idx].start = start;
        lineinfo[idx].end = size - 1;
        lineinfo[idx].len = size - start;
    }

    // Set the number of lines
    buffer->buff = buff;
    buffer->lineinfo = lineinfo;
    buffer->nline = nline;

    return 1;
}

// Free file buffer
void FreeBuff(buffer_t *buffer)
{
    if (!buffer) return;

    if (buffer->buff) free(buffer->buff);
    if (buffer->lineinfo) free(buffer->lineinfo);

    buffer->buff = NULL;
    buffer->lineinfo = NULL;
    buffer->nline = 0;
}

// Read observation data files (RINEX OBS, RTCM (TBD), UBX (TBD))
void ReadObsFiles(files_t *files, nav_t *nav, obss_t *obs)
{
    // Check if the files, navigation, and observation structures are valid
    if (!files || !nav || !obs) return;

    // Number of observation files
    int nfiles = files->n;

    // Get next available receiver index from current observation data
    int ridx = 1;
    if (obs->n > 0) {
        for (int i = 0; i < obs->n; i++) {
            if (obs->obs[i].rcv >= ridx) {
                ridx = obs->obs[i].rcv + 1;
            }
        }
    }

    // Loop through all observation files
    for (int i = 0; i < nfiles; i++) {

        // Get the file name
        const char *filename = GetFileName(files, i);
        if (!filename) continue;

        // Check if file name is the RINEX obs file name
        if (IsRinexObs(filename)) {

            // Read RINEX observation file
            if (ReadRnxObs(nav, obs, ridx, filename)) ridx++;
        }
    }

    // Remove duplicated data and sort observation data by time, receiver index,
    // and satellite index in ascending order
    SortObss(obs);
}

// Read navigation data files
void ReadNavFiles(files_t *files, nav_t *nav)
{
    // Check if the files and navigation structures are valid
    if (!files || !nav) return;

    // Number of navigation files
    int nfiles = files->n;

    // Loop through all navigation files
    for (int i = 0; i < nfiles; i++) {

        // Get the file name
        const char *filename = GetFileName(files, i);
        if (!filename) continue;

        // Check if file name is the RINEX navigation file name
        if (IsRinexNav(filename)) {

            // Read RINEX navigation file
            if (ReadRnxNav(nav, filename)) {
                continue;
            }
        }
    }

    // Remove duplicated data and sort ephemeris data by time transmission
    for (int i = 0; i < NSAT; i++) {
        SortEphs(&nav->ephs[i]);
    }

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
