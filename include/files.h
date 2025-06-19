// =============================================================================
// Files header
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#ifndef FILES_H
#define FILES_H

#ifdef __cplusplus
extern "C" {
#endif

// GNSS library
#include "types.h"          // for types

// =============================================================================
// Inline functions
// =============================================================================

// -----------------------------------------------------------------------------
// Get line from buffer
//
// args:
//  const buffer_t *buffer   (I) : buffer structure
//        size_t   l         (I) : line index
//        int      len       (O) : (optional) length of the line
//
// return:
//        char     *line     (O) : line string (NULL if error)
// -----------------------------------------------------------------------------
static inline char *GetLine(const buffer_t *buffer, size_t l, int *len)
{
    // Check if the buffer is valid
    if (!buffer || l >= buffer->nline) return NULL;

    // Get line from buffer
    char *line = buffer->buff + buffer->lineinfo[l].start;

    // Get length of the line
    if (len) *len = (int)buffer->lineinfo[l].len;

    // Return line
    return line;
}

// =============================================================================
// File functions
// =============================================================================

// -----------------------------------------------------------------------------
// Add file name to file string structure
//
// args:
//        files_t *files    (I,O) : file string structure
//  const char    *filename (I)   : file name to add
//
// return:
//        int     info      (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int AddFileName(files_t *files, const char *filename);

// -----------------------------------------------------------------------------
// Get file name from file string structure
//
// args:
//        files_t *files    (I) : file string structure
//        int     index     (I) : file index (0-based)
//
// return:
//  const char    *filename (O) : file name (NULL if error)
// -----------------------------------------------------------------------------
const char *GetFileName(const files_t *files, int index);

// -----------------------------------------------------------------------------
// Initialize file structure
//
// args:
//        file_t    *file   (I,O) : file structure
//
// return:
//        void              (-)   : no return value
// -----------------------------------------------------------------------------
void InitFile(file_t *file);

// -----------------------------------------------------------------------------
// Free file structure
//
// args:
//        file_t    *file   (I) : file structure
//
// return:
//        void              (-) : no return value
// -----------------------------------------------------------------------------
void FreeFile(file_t *file);

// =============================================================================
// Buffer functions
// =============================================================================

// -----------------------------------------------------------------------------
// Initialize file buffer structure
//
// args:
//        buffer_t *buffer (I,O) : buffer structure
//
// return:
//        int      info    (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int InitBuff(buffer_t *buffer);

// -----------------------------------------------------------------------------
// Get file buffer from file
//
// args:
//  const char      *filename (I)   : file name
//        buffer_t  *buffer   (O)   : buffer structure
//
// return:
//        int       info      (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int GetBuff(const char *filename, buffer_t *buffer);

// -----------------------------------------------------------------------------
// Free file buffer structure
//
// args:
//        buffer_t  *buffer (I) : buffer structure
//
// return:
//        void              (-) : no return value
// -----------------------------------------------------------------------------
void FreeBuff(buffer_t *buffer);

// =============================================================================
// File read functions
// =============================================================================

// -----------------------------------------------------------------------------
// Read observation data files (RINEX OBS, RTCM (TBD), UBX (TBD))
//
// args:
//       files_t *files (I)   : file string data structure
//       nav_t   *nav   (I,O) : navigation data structure
//       obss_t  *obs   (I,O) : observation data structure
//
// return:
//       void           (-)   : no return value
// -----------------------------------------------------------------------------
void ReadObsFiles(files_t *files, nav_t *nav, obss_t *obs);

// -----------------------------------------------------------------------------
// Read navigation data files
//
// args:
//       files_t *files (I)   : file string data structure
//       nav_t   *nav   (I,O) : navigation data structure
//
// return:
//       void           (-)   : no return value
// -----------------------------------------------------------------------------
void ReadNavFiles(files_t *files, nav_t *nav);

// -----------------------------------------------------------------------------
// Read DCB data files
//
// args:
//       files_t *files (I)   : file string data structure
//       nav_t   *nav   (I,O) : navigation data structure
//
// return:
//       void           (-)   : no return value
// -----------------------------------------------------------------------------
void ReadDcbFiles(files_t *files, nav_t *nav);

// -----------------------------------------------------------------------------
// Read antenna exchange data files
//
// args:
//       files_t *files (I)   : file string data structure
//       nav_t   *nav   (I,O) : navigation data structure
//
// return:
//       void           (-)   : no return value
// -----------------------------------------------------------------------------
void ReadAtxFiles(files_t *files, nav_t *nav);

// -----------------------------------------------------------------------------
// Read all files
//
// args:
//       file_t  *file  (I)   : file structure
//       nav_t   *nav   (I,O) : navigation data structure
//       obss_t  *obs   (I,O) : observation data structure
//
// return:
//       void           (-)   : no return value
// -----------------------------------------------------------------------------
void ReadFiles(file_t *file, nav_t *nav, obss_t *obs);

#ifdef __cplusplus
}
#endif

#endif
