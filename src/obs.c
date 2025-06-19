// =============================================================================
// Observation functions
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#include "obs.h"
#include "common.h"     // for GetFcn, Sat2Prn, Sys2Str, Str2Sys
#include "ephemeris.h"  // for GetEphType
#include <string.h>     // for strcmp, strncpy, strchr
#include <stdlib.h>     // for qsort
#include <math.h>       // for fabs

// =============================================================================
// Static type definitions
// =============================================================================

typedef struct fmap {       // Frequency band and attribute struct
    int  band;              // Frequency band index
    char *attr;             // Frequency attribute string
} fmap_t;

// =============================================================================
// Static global variables
// =============================================================================

// Band string
static const char BANDS[NBAND + 1] = "123456789";

// Observation code strings (* indicates not used signals)
static const char *OBSCODES[] = {
    "",        // NONE

    "L1D",     // BDS B1C(D)
    "L1P",     // GPS L1P*, GLO G1P*, BDS B1C(P)
  //"L1A",     // GAL E1(A)*
    "L1B",     // GAL E1(B), QZS L1Sb
    "L1C",     // GPS L1C/A, GLO G1C/A, GAL E1(C), QZS L1C/A, SBS L1C/A
    "L1S",     // GPS L1C(D), QZS L1C(D), BDS B1A(D)*
    "L1L",     // GPS L1C(P), QZS L1C(P), BDS B1A(P)*
    "L1X",     // GPS L1C(D+P), GAL E1(B+C), BDS B1C(D+P), QZS L1C(D+P)
    "L1Z",     // GAL E1(A+B+C)*, QZS L1S, BDS B1A(D+P)*

  //"L1Y",     // GPS L1Y*
  //"L1M",     // GPS L1M*
  //"L1W",     // GPS L1W*
  //"L1N",     // GPS L1N*

    "L2I",     // BDS B1(I)
    "L2Q",     // BDS B1(Q)*
    "L2C",     // GPS L2C/A*, GLO G2C/A
    "L2S",     // GPS L2C(M), QZS L2C(M)
    "L2L",     // GPS L2C(L), QZS L2C(L)
    "L2X",     // GPS L2C(M+L), BDS B1(I+Q), QZS L2C(M+L)

  //"L2P",     // GPS L2P*, GLO G2P*
  //"L2Y",     // GPS L2Y*
  //"L2M",     // GPS L2M*
    "L2W",     // GPS L2W
  //"L2D",     // GPS L2D*
  //"L2N",     // GPS L2N*

    "L3I",     // GLO G3(I)
    "L3Q",     // GLO G3(Q)
    "L3X",     // GLO G3(I+Q)

    "L4A",     // GLO G1A(OCd)
    "L4B",     // GLO G1A(OCp)
    "L4X",     // GLO G1A(OCd+OCp)

    "L5I",     // GPS L5(I), GAL E5A(I), QZS L5(I), SBS L5(I)
    "L5Q",     // GPS L5(Q), GAL E5A(Q), QZS L5(Q), SBS L5(Q)
    "L5D",     // BDS B2A(D), QZS L5S(I)
    "L5P",     // BDS B2A(P), QZS L5S(Q)
    "L5A",     // IRN L5(A)*
    "L5B",     // IRN L5(B)
    "L5C",     // IRN L5(C)
    "L5X",     // GPS L5(I+Q), GAL E5A(I+Q), BDS B2A(D+P), IRN L5(B+C), QZS L5(I+Q), SBS L5(I+Q)
    "L5Z",     // QZS L5S(I+Q)

    "L6I",     // BDS B3(I)
  //"L6Q",     // BDS B3(Q)*
  //"L6D",     // BDS B3A(D)*
  //"L6P",     // BDS B3A(P)*
    "L6A",     // GLO G2A(CSI), GAL E6(A)*
    "L6B",     // GLO G2A(OCp), GAL E6(B)
    "L6C",     // GAL E6(C)
    "L6S",     // QZS L6(D)
    "L6L",     // QZS L6(P)
    "L6E",     // QZS L6(E)
    "L6X",     // GLO G2A(CSI+OCp), GAL E6(B+C), BDS B3(I+Q), QZS L6(D+P)
    "L6Z",     // GAL E6(A+B+C)*, BDS B3A(D+P)*, QZS L6(D+E)

    "L7I",     // GAL E5B(I), BDS B2(I)
    "L7Q",     // GAL E5B(Q), BDS B2(Q)
    "L7D",     // BDS B2B(D)
    "L7P",     // BDS B2B(P)
    "L7X",     // GAL E5B(I+Q), BDS B2(I+Q)
    "L7Z",     // BDS B2B(D+P)

    "L8I",     // GAL E5AB(I)
    "L8Q",     // GAL E5AB(Q)
    "L8D",     // BDS B2AB(D)
    "L8P",     // BDS B2AB(P)
    "L8X",     // GAL E5AB(I+Q), BDS B2AB(D+P)

  //"L9A",     // IRN S(A)*
    "L9B",     // IRN S(B)
    "L9C",     // IRN S(C)
    "L9X"      // IRN S(B+C)
};

// GPS frequency band and attribute mapping
static const fmap_t FMAP_GPS[] = {
    {1, "C"},           // GPS L1C/A
    {2, "W"},           // GPS L2W
    {5, "IQX"},         // GPS L5  (I,Q,X)
    {1, "SLX"},         // GPS L1C (S,L,X)
    {2, "SLX"},         // GPS L2C (S,L,X)
};

// GLO frequency band and attribute mapping
static const fmap_t FMAP_GLO[] = {
    {1, "C"},           // GLO G1C
    {2, "C"},           // GLO G2C
    {3, "IQX"},         // GLO G3  (I,Q,X)
    {4, "ABX"},         // GLO G1A (A,B,X)
    {6, "ABX"},         // GLO G2A (A,B,X)
};

// GAL frequency band and attribute mapping for F/NAV
static const fmap_t FMAP_GAL_FNAV[] = {
    {1, "BCX"},         // GAL E1   (B,C,X)
    {5, "IQX"},         // GAL E5A  (I,Q,X)
    {7, "IQX"},         // GAL E5B  (I,Q,X)
    {6, "BCX"},         // GAL E6   (B,C,X)
    {8, "IQX"},         // GAL E5AB (I,Q,X)
};

// GAL frequency band and attribute mapping for I/NAV
static const fmap_t FMAP_GAL_INAV[] = {
    {1, "BCX"},         // GAL E1   (B,C,X)
    {7, "IQX"},         // GAL E5B  (I,Q,X)
    {5, "IQX"},         // GAL E5A  (I,Q,X)
    {6, "BCX"},         // GAL E6   (B,C,X)
    {8, "IQX"},         // GAL E5AB (I,Q,X)
};

// BDS frequency band and attribute mapping
static const fmap_t FMAP_BDS[] = {
    {2, "I"},           // BDS B1   (I)
    {6, "I"},           // BDS B3   (I)
    {7, "I"},           // BDS B2   (I)
    {1, "DPX"},         // BDS B1C  (D, P, X)
    {7, "DPZ"},         // BDS B2B  (D, P, Z)
    {5, "DPX"},         // BDS B2A  (D, P, X)
    {8, "DPX"},         // BDS B2AB (D, P, X)
};

// QZS frequency band and attribute mapping
static const fmap_t FMAP_QZS[] = {
    {1, "C"},           // QZS L1C/A
    {2, "SLX"},         // QZS L2C (S,L,X)
    {5, "IQX"},         // QZS L5  (I,Q,X)
    {6, "SLX"},         // QZS L6  (S,L,X)
    {1, "SLX"},         // QZS L1C (S,L,X)
    {5, "DPZ"},         // QZS L5S (D,P,Z)
};

// IRN frequency band and attribute mapping
static const fmap_t FMAP_IRN[] = {
    {5, "BCX"},         // IRN L5  (B,C,X)
    {9, "BCX"},         // IRN S   (B,C,X)
};

// SBS frequency band and attribute mapping
static const fmap_t FMAP_SBS[] = {
    {1, "C"},           // SBS L1C/A
    {5, "IQX"},         // SBS L5  (I,Q,X)
};



// =============================================================================
// Global variables
// =============================================================================

// The number of obs codes
const int NCODE = sizeof(OBSCODES) / sizeof(OBSCODES[0]) - 1;

// =============================================================================
// Static functions (internal use only)
// =============================================================================

// -----------------------------------------------------------------------------
// Convert GPS frequency band index to frequency (static function)
//
// args:
//        int   band    (I) : frequency band index
//
// return:
//        double freq   (O) : carrier frequency [Hz] (0.0 if error)
// -----------------------------------------------------------------------------
static double Band2Freq_GPS(int band)
{
    switch (band) {
        case 1: return FREQ_L1;
        case 2: return FREQ_L2;
        case 5: return FREQ_L5;
        default: return 0.0;
    }
}

// -----------------------------------------------------------------------------
// Convert GLO frequency band index to frequency (static function)
//
// args:
//        int   band    (I) : frequency band index
//        int   prn     (I) : GLONASS satellite PRN number
//
// return:
//        double freq   (O) : carrier frequency [Hz] (0.0 if error)
// -----------------------------------------------------------------------------
static double Band2Freq_GLO(int band, int prn)
{
    int fcn;
    if (!(GetFcn(prn, &fcn))) return 0.0;

    switch (band) {
        case 1: return FREQ_G1 + DFREQ_G1 * fcn;
        case 2: return FREQ_G2 + DFREQ_G2 * fcn;
        case 3: return FREQ_G3;
        case 4: return FREQ_G1A;
        case 6: return FREQ_G2A;
        default: return 0.0;
    }
}

// -----------------------------------------------------------------------------
// Convert GAL frequency band index to frequency (static function)
//
// args:
//        int   band    (I) : frequency band index
//
// return:
//        double freq   (O) : carrier frequency [Hz] (0.0 if error)
// -----------------------------------------------------------------------------
static double Band2Freq_GAL(int band)
{
    switch (band) {
        case 1: return FREQ_E1;
        case 5: return FREQ_E5A;
        case 7: return FREQ_E5B;
        case 8: return FREQ_E5AB;
        case 6: return FREQ_E6;
        default: return 0.0;
    }
}

// -----------------------------------------------------------------------------
// Convert BDS frequency band index to frequency (static function)
//
// args:
//        int   band    (I) : frequency band index
//
// return:
//        double freq   (O) : carrier frequency [Hz] (0.0 if error)
// -----------------------------------------------------------------------------
static double Band2Freq_BDS(int band)
{
    switch (band) {
        case 2: return FREQ_B1;
        case 1: return FREQ_B1C;
        case 5: return FREQ_B2A;
        case 7: return FREQ_B2B;
        case 8: return FREQ_B2AB;
        case 6: return FREQ_B3;
        default: return 0.0;
    }
}

// -----------------------------------------------------------------------------
// Convert QZS frequency band index to frequency (static function)
//
// args:
//        int   band    (I) : frequency band index
//
// return:
//        double freq   (O) : carrier frequency [Hz] (0.0 if error)
// -----------------------------------------------------------------------------
static double Band2Freq_QZS(int band)
{
    switch (band) {
        case 1: return FREQ_L1;
        case 2: return FREQ_L2;
        case 5: return FREQ_L5;
        case 6: return FREQ_L6;
        default: return 0.0;
    }
}

// -----------------------------------------------------------------------------
// Convert IRN frequency band index to frequency (static function)
//
// args:
//        int   band    (I) : frequency band index
//
// return:
//        double freq   (O) : carrier frequency [Hz] (0.0 if error)
// -----------------------------------------------------------------------------
static double Band2Freq_IRN(int band)
{
    switch (band) {
        case 5: return FREQ_L5;
        case 9: return FREQ_S;
        default: return 0.0;
    }
}

// -----------------------------------------------------------------------------
// Convert SBS frequency band index to frequency (static function)
//
// args:
//        int   band    (I) : frequency band index
//
// return:
//        double freq   (O) : carrier frequency [Hz] (0.0 if error)
// -----------------------------------------------------------------------------
static double Band2Freq_SBS(int band)
{
    switch (band) {
        case 1: return FREQ_L1;
        case 5: return FREQ_L5;
        default: return 0.0;
    }
}

// -----------------------------------------------------------------------------
// Convert observation code index of GPS to frequency index
//
// Index 1 - GPS L1C/A
// Index 2 - GPS L2W
// Index 3 - GPS L5  (I,Q,X)
// Index 4 - GPS L1C (S,L,X)
// Index 5 - GPS L2C (S,L,X)
//
// args:
//        int   code    (I) : observation code index
//
// return:
//        int   fidx    (O) : frequency index (0 if error)
// -----------------------------------------------------------------------------
static int Code2Fidx_GPS(int code)
{
    // Convert observation code index to string
    codeStr_t codeStr = Code2Str(code);

    // Check if the observation code is valid
    if (codeStr.str[0] != 'L') return 0;

    // Maximum number of frequency index
    int maxf = sizeof(FMAP_GPS) / sizeof(FMAP_GPS[0]);

    for (int f = 0; f < NFREQ && f < maxf; f++) {

        // Check band index
        if (Str2Band(codeStr.str[1]) != FMAP_GPS[f].band) continue;

        // Check attribute (one of the attributes must match)
        if (strchr(FMAP_GPS[f].attr, codeStr.str[2]) == NULL) continue;

        // Return frequency index
        return f + 1;
    }

    return 0;
}

// -----------------------------------------------------------------------------
// Convert observation code index of GLO to frequency index
//
// Index 1 - G1C
// Index 2 - G2C
// Index 3 - G3  (I, Q, X)
// Index 4 - G1A (A, B, X)
// Index 5 - G2A (A, B, X)
//
// args:
//        int   code    (I) : observation code index
//
// return:
//        int   fidx    (O) : frequency index (0 if error)
// -----------------------------------------------------------------------------
static int Code2Fidx_GLO(int code)
{
    // Convert observation code index to string
    codeStr_t codeStr = Code2Str(code);

    // Check if the observation code is valid
    if (codeStr.str[0] != 'L') return 0;

    // Maximum number of frequency index
    int maxf = sizeof(FMAP_GLO) / sizeof(FMAP_GLO[0]);

    for (int f = 0; f < NFREQ && f < maxf; f++) {

        // Check band index
        if (Str2Band(codeStr.str[1]) != FMAP_GLO[f].band) continue;

        // Check attribute (one of the attributes must match)
        if (strchr(FMAP_GLO[f].attr, codeStr.str[2]) == NULL) continue;

        // Return frequency index
        return f + 1;
    }

    return 0;
}

// -----------------------------------------------------------------------------
// Convert observation code index of GAL to frequency index
//
// I/NAV mode
//      Index 1 - E1  (B, C, X)
//      Index 2 - E5B (I, Q, X)
//      Index 3 - E5A (I, Q, X)
//      Index 4 - E6  (B, C, X)
//      Index 5 - E5AB(I, Q, X)
//
// F/NAV mode
//      Index 1 - E1  (B, C, X)
//      Index 2 - E5A (I, Q, X)
//      Index 3 - E5B (I, Q, X)
//      Index 4 - E6  (B, C, X)
//      Index 5 - E5AB(I, Q, X)
//
// args:
//        int   code    (I) : observation code index
//
// return:
//        int   fidx    (O) : frequency index (0 if error)
// -----------------------------------------------------------------------------
static int Code2Fidx_GAL(int code)
{

    // Convert observation code index to string
    codeStr_t codeStr = Code2Str(code);

    // Check if the observation code is valid
    if (codeStr.str[0] != 'L') return 0;

    // Check if the ephemeris type is valid
    int sysType = GetEphType(Str2Sys('E'));
    if (sysType < 0 || sysType > 1) return 0;

    if (sysType == 0) {
        // I/NAV mode

        // Maximum number of frequency index
        int maxf = sizeof(FMAP_GAL_INAV) / sizeof(FMAP_GAL_INAV[0]);

        for (int f = 0; f < NFREQ && f < maxf; f++) {

            // Check band index
            if (Str2Band(codeStr.str[1]) != FMAP_GAL_INAV[f].band) continue;

            // Check attribute (one of the attributes must match)
            if (strchr(FMAP_GAL_INAV[f].attr, codeStr.str[2]) == NULL) continue;

            // Return frequency index
            return f + 1;
        }
    }
    else {
        // F/NAV mode

        // Maximum number of frequency index
        int maxf = sizeof(FMAP_GAL_FNAV) / sizeof(FMAP_GAL_FNAV[0]);

        for (int f = 0; f < NFREQ && f < maxf; f++) {

            // Check band index
            if (Str2Band(codeStr.str[1]) != FMAP_GAL_FNAV[f].band) continue;

            // Check attribute (one of the attributes must match)
            if (strchr(FMAP_GAL_FNAV[f].attr, codeStr.str[2]) == NULL) continue;

            // Return frequency index
            return f + 1;
        }
    }

    return 0;
}

// -----------------------------------------------------------------------------
// Convert observation code index of BDS to frequency index
//
// Index 1 - B1     (I)
// Index 2 - B3     (I)
// Index 3 - B2     (I)
// Index 4 - B1C    (D, P, X)
// Index 5 - B2B    (D, P, Z)
// Index 6 - B2A    (D, P, X)
// Index 7 - B2AB   (D, P, X)
//
// args:
//        int   code    (I) : observation code index
//
// return:
//        int   fidx    (O) : frequency index (0 if error)
// -----------------------------------------------------------------------------
static int Code2Fidx_BDS(int code)
{
    // Convert observation code index to string
    codeStr_t codeStr = Code2Str(code);

    // Check if the observation code is valid
    if (codeStr.str[0] != 'L') return 0;

    // Maximum number of frequency index
    int maxf = sizeof(FMAP_BDS) / sizeof(FMAP_BDS[0]);

    for (int f = 0; f < NFREQ && f < maxf; f++) {

        // Check band index
        if (Str2Band(codeStr.str[1]) != FMAP_BDS[f].band) continue;

        // Check attribute (one of the attributes must match)
        if (strchr(FMAP_BDS[f].attr, codeStr.str[2]) == NULL) continue;

        // Return frequency index
        return f + 1;
    }

    return 0;
}

// -----------------------------------------------------------------------------
// Convert observation code index of QZS to frequency index
//
// Index 1 - L1C/A (B, C)
// Index 2 - L2C   (S, L, X)
// Index 3 - L5    (I, Q, X)
// Index 4 - L6    (S, L, E, X, Z)
// Index 5 - L1C   (S, L, X)
// Index 6 - L5S   (D, P, Z)
//
// args:
//        int   code    (I) : observation code index
//
// return:
//        int   fidx    (O) : frequency index (0 if error)
// -----------------------------------------------------------------------------
static int Code2Fidx_QZS(int code)
{
    // Convert observation code index to string
    codeStr_t codeStr = Code2Str(code);

    // Check if the observation code is valid
    if (codeStr.str[0] != 'L') return 0;

    // Maximum number of frequency index
    int maxf = sizeof(FMAP_QZS) / sizeof(FMAP_QZS[0]);

    for (int f = 0; f < NFREQ && f < maxf; f++) {

        // Check band index
        if (Str2Band(codeStr.str[1]) != FMAP_QZS[f].band) continue;

        // Check attribute (one of the attributes must match)
        if (strchr(FMAP_QZS[f].attr, codeStr.str[2]) == NULL) continue;

        // Return frequency index
        return f + 1;
    }

    return 0;
}

// -----------------------------------------------------------------------------
// Convert observation code index of IRN to frequency index
//
// Index 1 - L5 (B, C, X)
// Index 2 - S  (B, C, X)
//
// args:
//        int   code    (I) : observation code index
//
// return:
//        int   fidx    (O) : frequency index (0 if error)
// -----------------------------------------------------------------------------
static int Code2Fidx_IRN(int code)
{
    // Convert observation code index to string
    codeStr_t codeStr = Code2Str(code);

    // Check if the observation code is valid
    if (codeStr.str[0] != 'L') return 0;

    // Maximum number of frequency index
    int maxf = sizeof(FMAP_IRN) / sizeof(FMAP_IRN[0]);

    for (int f = 0; f < NFREQ && f < maxf; f++) {

        // Check band index
        if (Str2Band(codeStr.str[1]) != FMAP_IRN[f].band) continue;

        // Check attribute (one of the attributes must match)
        if (strchr(FMAP_IRN[f].attr, codeStr.str[2]) == NULL) continue;

        // Return frequency index
        return f + 1;
    }

    return 0;
}

// -----------------------------------------------------------------------------
// Convert observation code index of SBS to frequency index
//
// Index 1 - L1C/A (C)
// Index 2 - L5    (I, Q, X)
//
// args:
//        int   code    (I) : observation code index
//
// return:
//        int   fidx    (O) : frequency index (0 if error)
// -----------------------------------------------------------------------------
static int Code2Fidx_SBS(int code)
{
    // Convert observation code index to string
    codeStr_t codeStr = Code2Str(code);

    // Check if the observation code is valid
    if (codeStr.str[0] != 'L') return 0;

    // Maximum number of frequency index
    int maxf = sizeof(FMAP_SBS) / sizeof(FMAP_SBS[0]);

    for (int f = 0; f < NFREQ && f < maxf; f++) {

        // Check band index
        if (Str2Band(codeStr.str[1]) != FMAP_SBS[f].band) continue;

        // Check attribute (one of the attributes must match)
        if (strchr(FMAP_SBS[f].attr, codeStr.str[2]) == NULL) continue;

        // Return frequency index
        return f + 1;
    }

    return 0;
}

// Resize observation data set structure
static int ResizeObss(obss_t *obss, int nnew)
{
    // Check if the observation data set structure is valid
    if (!obss || nnew <= 0) return 0;

    obs_t *newObs = (obs_t *)realloc(obss->obs, nnew * sizeof(obs_t));
    if (!newObs) return 0;

    obss->obs = newObs;
    obss->nmax = nnew;

    return 1;
}

// Compare observation data by time, receiver index, satellite index
static int CompareObs(const void *a, const void *b)
{
    const obs_t *obs1 = (const obs_t *)a;
    const obs_t *obs2 = (const obs_t *)b;

    // Time difference
    double dt = obs1->time - obs2->time;
    if (fabs(dt) > 1e-9) return (dt > 0.0) ? 1 : -1;

    // Receiver index
    if (obs1->rcv != obs2->rcv) return obs1->rcv - obs2->rcv;

    // Satellite index
    return obs1->sat - obs2->sat;
}

// =============================================================================
// Observation data structure functions
// =============================================================================

// Initialize observation data set structure
void InitObss(obss_t *obss)
{
    // Check if the observation data set structure is valid
    if (!obss) return;

    // Initialize the observation data set structure
    obss->n = 0;
    obss->nmax = 0;
    obss->obs = NULL;
}

// Free observation data set structure
void FreeObss(obss_t *obss)
{
    // Check if the observation data set structure is valid
    if (!obss) return;

    // Free the observation data set structure
    if (obss->obs) {
        free(obss->obs);
        obss->obs = NULL;
    }
    obss->n = obss->nmax = 0;
}

// Add observation data to observation data set
int AddObs(obss_t *obss, const obs_t *obs)
{
    // Check if the observation data set structure is valid
    if (!obss || !obs) return 0;

    // Check if the observation data set is full
    if (obss->n >= obss->nmax) {
        int nnew = (obss->nmax == 0) ? 2 : obss->nmax * 2;
        if (!ResizeObss(obss, nnew)) return 0;
    }

    // Add the observation data to the observation data set
    obss->obs[obss->n] = *obs;
    obss->n++;

    return 1;
}

// Sort observation data set by the order of time, receiver index, satellite index
void SortObss(obss_t *obss)
{
    // Check if the observation data set structure is valid
    if (!obss || obss->n == 0) return;

    // Sort the observation data set by the order of time, receiver index, satellite index
    qsort(obss->obs, obss->n, sizeof(obs_t), CompareObs);

    // Delete duplicate observation data
    int n = 0;
    for (int i = 0; i < obss->n; i++) {

        // Check if the observation data is the same as previous one
        if (i > 0 &&
            obss->obs[i].time == obss->obs[i-1].time &&
            obss->obs[i].rcv  == obss->obs[i-1].rcv  &&
            obss->obs[i].sat  == obss->obs[i-1].sat) {
            continue;  // Skip duplicate
        }

        // Copy the observation data to the next position
        if (n != i) {
            obss->obs[n] = obss->obs[i];
        }
        n++;
    }

    // Update the number of observation data
    obss->n = n;
}


// =============================================================================
// Observation code conversion functions
// =============================================================================

// Convert observation code string to index
int Str2Code(codeStr_t codeStr)
{
    for (int i = 1; i <= NCODE; i++) {
        if (!strcmp(codeStr.str, OBSCODES[i])) return i;
    }
    return 0;
}

// Convert observation code index to string
codeStr_t Code2Str(int code)
{
    codeStr_t result = {0};
    if (code <= 0 || code > NCODE) return result;

    // Copy the observation code string to struct
    strncpy(result.str, OBSCODES[code], CODE_STR_SIZE - 1);
    result.str[CODE_STR_SIZE - 1] = '\0';  // null terminator

    return result;
}

// Convert observation code index to frequency band index
int Code2Band(int code)
{
    // Check if the observation code index is valid
    if (code <= 0 || code > NCODE) return 0;

    // Get the frequency band index
    return Str2Band(OBSCODES[code][1]);
}

// Convert observation code index with satellite index to carrier-frequency
double Code2Freq(int sat, int code)
{
    // Check if the satellite index is valid
    if (sat <= 0 || sat > NSAT) return 0.0;

    // Check if the observation code index is valid
    if (code <= 0 || code > NCODE) return 0.0;

    // Get the frequency band index
    int band = Code2Band(code);

    // Get the carrier frequency
    return Band2Freq(sat, band);
}

// Convert observation code index with system to frequency index
int Code2Fidx(int sys, int code)
{
    switch (Sys2Str(sys)) {
        case STR_GPS: return Code2Fidx_GPS(code);
        case STR_GLO: return Code2Fidx_GLO(code);
        case STR_GAL: return Code2Fidx_GAL(code);
        case STR_BDS: return Code2Fidx_BDS(code);
        case STR_QZS: return Code2Fidx_QZS(code);
        case STR_IRN: return Code2Fidx_IRN(code);
        case STR_SBS: return Code2Fidx_SBS(code);
        default: return 0;
    }
}

// =============================================================================
// Frequency band conversion functions
// =============================================================================

// Convert frequency band index to frequency band string
char Band2Str(int band)
{
    // Check if the band index is valid
    if (band <= 0 || band > NBAND) return 0;
    return BANDS[band-1];
}

// Convert frequency band string to index
int Str2Band(char band)
{
    // Check if the band character is valid (null character)
    if (!band) return 0;

    // Find the character in BANDS array
    char *found = strchr(BANDS, band);

    // Check if the character was found
    if (!found) return 0;

    // Calculate the index
    return found - BANDS + 1;
}

// Convert frequency band index with satellite index to carrier-frequency
double Band2Freq(int sat, int band)
{
    // Check if the satellite index is valid
    int sys, prn;
    if ((sys = Sat2Prn(sat, &prn)) == 0) return 0.0;

    // Check if the frequency band index is valid
    if (band <= 0 || band > NBAND) return 0.0;

    switch (Sys2Str(sys)) {
        case STR_GPS: return Band2Freq_GPS(band);
        case STR_GLO: return Band2Freq_GLO(band, prn); // It needs PRN due to the frequency channel number
        case STR_GAL: return Band2Freq_GAL(band);
        case STR_BDS: return Band2Freq_BDS(band);
        case STR_QZS: return Band2Freq_QZS(band);
        case STR_IRN: return Band2Freq_IRN(band);
        case STR_SBS: return Band2Freq_SBS(band);
        default: return 0.0;
    }
}

// Convert frequency index with system index to frequency band index
int Fidx2Band(int sys, int fidx)
{
    // Check if the system index is valid
    if (sys <= 0 || sys > NSYS) return 0;

    // Check if the frequency index is valid
    if (fidx <= 0 || fidx > NFREQ) return 0;

    // FMAP pointer and size
    const fmap_t *fmap = NULL;
    int maxf = 0;

    switch (Sys2Str(sys)) {
        case STR_GPS:
            fmap = FMAP_GPS;
            maxf = sizeof(FMAP_GPS) / sizeof(FMAP_GPS[0]);
            break;
        case STR_GLO:
            fmap = FMAP_GLO;
            maxf = sizeof(FMAP_GLO) / sizeof(FMAP_GLO[0]);
            break;
        case STR_GAL:
            if (GetEphType(Sys2Str('E')) == 0) {
                fmap = FMAP_GAL_INAV;
                maxf = sizeof(FMAP_GAL_INAV) / sizeof(FMAP_GAL_INAV[0]);
            } else {
                fmap = FMAP_GAL_FNAV;
                maxf = sizeof(FMAP_GAL_FNAV) / sizeof(FMAP_GAL_FNAV[0]);
            }
            break;
        case STR_BDS:
            fmap = FMAP_BDS;
            maxf = sizeof(FMAP_BDS) / sizeof(FMAP_BDS[0]);
            break;
        case STR_QZS:
            fmap = FMAP_QZS;
            maxf = sizeof(FMAP_QZS) / sizeof(FMAP_QZS[0]);
            break;
        case STR_IRN:
            fmap = FMAP_IRN;
            maxf = sizeof(FMAP_IRN) / sizeof(FMAP_IRN[0]);
            break;
        case STR_SBS:
            fmap = FMAP_SBS;
            maxf = sizeof(FMAP_SBS) / sizeof(FMAP_SBS[0]);
            break;
        default:
            return 0;
    }

    // Validate frequency index range
    if (fidx > maxf) return 0;

    return fmap[fidx-1].band;
}

// =============================================================================
// End of file
// =============================================================================
