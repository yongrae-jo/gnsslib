// =============================================================================
// Observable header
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#ifndef OBS_H
#define OBS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "const.h"                      // for NFREQ

// =============================================================================
// Macros
// =============================================================================

#define CODE_STR_SIZE 4  // Number of characters in observation code string (LXX)

// =============================================================================
// Global variables
// =============================================================================

extern const int NCODE;    // The number of obs codes

// =============================================================================
// Type definition
// =============================================================================

typedef struct obs {                    // Struct of observation data
    double time;                        // Standard time (GPST)
    int    rcv;                         // Receiver index
    int    sat;                         // Satellite index
    int    code[NFREQ];                 // Observation code index
    double P   [NFREQ];                 // Pseudorange [m]
    double L   [NFREQ];                 // Carrier phase [m]
    double D   [NFREQ];                 // Doppler frequency [Hz]
    double SNR [NFREQ];                 // Signal-to-noise ratio [dB]
    int    LLI [NFREQ];                 // Loss of lock indicator
} obs_t;

typedef struct obss {                  // Struct of observation data set
    int    n, nmax;                    // Number of observations/allocated memory
    obs_t  *obs;                       // Observation data
} obss_t;

typedef struct {
    char str[CODE_STR_SIZE];
} codeStr_t;

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
