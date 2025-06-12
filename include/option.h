// =============================================================================
// Option header
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#ifndef OPTION_H
#define OPTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "const.h"                      // NSAT

// =============================================================================
// Type definition
// =============================================================================

typedef struct opt {                    // Struct of option data
    int mode;                           // Processing mode
    int engine;                         // Processing engine
    int nrcv;                           // Number of receivers
    int nfreq;                          // Number of frequencies

    int ephopt;                         // Ephemeris option
    int posopt;                         // Position option
    int ionoopt;                        // Ionospheric option
    int tropoopt;                       // Tropospheric option
    int par;                            // Partial ambiguity resolution
    int cascade;                        // Cascading ambiguity resolution
    int gloaropt;                       // GLONASS ambiguity resolution
    int dynamics;                       // Receiver dynamics option

    int maxout;                         // Maximum outage count to reset state
    int minlock;                        // Minimum lock count to fix ambiguity

    double ts;                          // Processing time start [s] (0.0: all)
    double te;                          // Processing time end [s] (0.0: all)

    double err;                         // Carrier phase measurement error std [m] (zenith direction)
    double errratio;                    // Pseudorange measurement error ratio;

    // Process noise in EKF (0.0: unlinked in time)
    double procnoiseAmb;                // Phase ambiguity [cycle]
    double procnoiseTropo;              // Zenith wet delay [m]
    double procnoiseIono;               // Ionospheric delay [m]
    double procnoiseHacc;               // Horizontal acceleration [m/s^2]
    double procnoiseVacc;               // Vertical acceleration [m/s^2]
    double procnoiseDtr;                // Receiver clock [m]
    double procnoiseDts;                // Satellite clock [m]
    double procnoiseIsb;                // Inter-system bias [m]
    double procnoiseCbr;                // Receiver code bias [m]
    double procnoiseCbs;                // Satellite code bias [m]
    double procnoisePbr;                // Receiver phase bias [m]
    double procnoisePbs;                // Satellite phase bias [m]

    double elmask;                      // Elevation mask angle [rad]
    double maxgdop;                     // Maximum GDOP

    int exsats[NSAT];                   // Excluded satellites (!0: excluded)
} opt_t;

// =============================================================================
// Option functions
// =============================================================================

// -----------------------------------------------------------------------------
// Set default option settings
//
// args:
//      opt_t *opt (I/O) : Option data
// -----------------------------------------------------------------------------
void SetDefaultOpt(opt_t *opt);


#ifdef __cplusplus
}
#endif

#endif
