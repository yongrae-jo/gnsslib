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

#include "common.h"

// =============================================================================
// Macros
// =============================================================================

#define MAX_FILE_NAME_LEN 1024          // Maximum number of characters in file name

// =============================================================================
// Type definition
// =============================================================================

typedef struct files {                  // Struct of file name string array
    char **names;                       // Dynamic array of file names
    int  n;                             // Current number of files
    int  nmax;                          // Maximum number of files (allocated)
} files_t;

typedef struct file {                   // Struct of files
    files_t obsfiles;                   // Observation files
    files_t navfiles;                   // Navigation files
    files_t sp3files;                   // SP3 files
    files_t clkfiles;                   // Clock files
    files_t dcbfiles;                   // DCB files
    files_t atxfiles;                   // Antenna exchange files
} file_t;

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
// File read functions
// =============================================================================

// -----------------------------------------------------------------------------
// Read observation data files
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
