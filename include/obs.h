// =============================================================================
// Observable header
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#ifndef OBS_H
#define OBS_H

// GNSS library
#include "types.h"          // for types

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Observation data structure functions
// =============================================================================

// -----------------------------------------------------------------------------
// Initialize observation data set structure
//
// args:
//        obss_t *obss (I,O) : observation data set structure
//
// return:
//        void         (-)   : no return value
// -----------------------------------------------------------------------------
void InitObss(obss_t *obss);

// -----------------------------------------------------------------------------
// Free observation data set structure
//
// args:
//        obss_t *obss (I) : observation data set structure
//
// return:
//        void         (-) : no return value
// -----------------------------------------------------------------------------
void FreeObss(obss_t *obss);

// -----------------------------------------------------------------------------
// Add observation data to observation data set
//
// args:
//        obss_t *obss (I,O) : observation data set structure
//  const obs_t  *obs  (I)   : observation data to add
//
// return:
//        int    info  (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int AddObs(obss_t *obss, const obs_t *obs);

// -----------------------------------------------------------------------------
// Sort observation data set by the order of time, receiver index, satellite
// index
//
// args:
//        obss_t *obss (I,O) : observation data set structure
//
// return:
//        void         (-)   : no return value
// -----------------------------------------------------------------------------
void SortObss(obss_t *obss);

// =============================================================================
// Observation code conversion functions
// =============================================================================

// -----------------------------------------------------------------------------
// Convert observation code string to index
//
// args:
//        codestr_t codestr (I) : observation code string (LXX format)
//
// return:
//        int       code    (O) : observation code index (0 if not found)
// -----------------------------------------------------------------------------
int Str2Code(codeStr_t codeStr);

// -----------------------------------------------------------------------------
// Convert observation code index to string
//
// args:
//        int   code    (I) : observation code index
//
// return:
//        codestr_t result (O) : observation code string (LXX format, empty if error)
// -----------------------------------------------------------------------------
codeStr_t Code2Str(int code);

// -----------------------------------------------------------------------------
// Convert observation code index to frequency band index
//
// args:
//        int   code    (I) : observation code index
//
// return:
//        int   band    (O) : frequency band index (0 if error)
// -----------------------------------------------------------------------------
int Code2Band(int code);

// -----------------------------------------------------------------------------
// Convert observation code index with satellite index to carrier-frequency
//
// args:
//        int   sat     (I) : satellite index
//        int   code    (I) : observation code index
//
// return:
//        double freq   (O) : carrier frequency [Hz] (0.0 if error)
// -----------------------------------------------------------------------------
double Code2Freq(int sat, int code);

// -----------------------------------------------------------------------------
// Convert observation code index to frequency index
//
// args:
//        int   code    (I) : observation code index
//
// return:
//        int   fidx    (O) : frequency index (0 if error)
// -----------------------------------------------------------------------------
int Code2Fidx(int sys, int code);

// =============================================================================
// Frequency band conversion functions
// =============================================================================

// -----------------------------------------------------------------------------
// Convert frequency band index to frequency band string
//
// args:
//        int   band    (I) : frequency band index
//
// return:
//        char  str     (O) : frequency band character (0 if error)
// -----------------------------------------------------------------------------
char Band2Str(int band);

// -----------------------------------------------------------------------------
// Convert frequency band string to index
//
// args:
//        char  band    (I) : frequency band character
//
// return:
//        int   index   (O) : frequency band index (0 if error)
// -----------------------------------------------------------------------------
int Str2Band(char band);

// -----------------------------------------------------------------------------
// Convert frequency band index with satellite index to carrier-frequency
//
// args:
//        int   sat     (I) : satellite index
//        int   band    (I) : frequency band index
//
// return:
//        double freq   (O) : carrier frequency [Hz] (0.0 if error)
// -----------------------------------------------------------------------------
double Band2Freq(int sat, int band);

// -----------------------------------------------------------------------------
// Convert frequency index with system index to frequency band index
//
// args:
//        int   sys     (I) : system index
//        int   fidx    (I) : frequency index
//
// return:
//        int   band    (O) : frequency band index (0 if error)
// -----------------------------------------------------------------------------
int Fidx2Band(int sys, int fidx);

// =============================================================================
// End of header
// =============================================================================
#ifdef __cplusplus
}
#endif

#endif // OBS_H
