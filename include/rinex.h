// =============================================================================
// RINEX file function header
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#ifndef RINEX_H
#define RINEX_H

// GNSS library
#include "types.h"

// =============================================================================
// RINEX check functions
// =============================================================================

// -----------------------------------------------------------------------------
// Check if file name is the RINEX obs file name
//
// args:
// const char    *filename (I)   : file name
//
// return:
//       int     info      (O)   : 1 if RINEX obs file, 0 if not
// -----------------------------------------------------------------------------
int IsRinexObs(const char *filename);

// -----------------------------------------------------------------------------
// Check if file name is the RINEX navigation file name
//
// args:
// const char    *filename (I)   : file name
//
// return:
//       int     info      (O)   : 1 if RINEX navigation file, 0 if not
// -----------------------------------------------------------------------------
int IsRinexNav(const char *filename);

// =============================================================================
// RINEX file read functions
// =============================================================================

// -----------------------------------------------------------------------------
// Read RINEX observation file (support version 2.xx and 3.xx)
//
// args:
//       nav_t   *nav      (I,O) : navigation data structure
//       obss_t  *obs      (I,O) : observation data structure
//       int     rcvidx    (I)   : receiver index
// const char    *filename (I)   : file name
//
// return:
//       int     info      (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int ReadRnxObs(nav_t *nav, obss_t *obs, int rcvidx, const char *filename);

// -----------------------------------------------------------------------------
// Read RINEX navigation file (support version 2.xx and 3.xx)
//
// args:
//       nav_t   *nav      (I,O) : navigation data structure
// const char    *filename (I)   : file name
//
// return:
//       int     info      (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int ReadRnxNav(nav_t *nav, const char *filename);

#endif // RINEX_H
