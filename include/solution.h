// =============================================================================
// Solution header file
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#ifndef SOLUTION_H
#define SOLUTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"

// =============================================================================
// Type definitions
// =============================================================================

typedef struct sol {                    // Struct of solution data
    double time;                        // Solution time (GPST) (standard time)

    int    fix;                         // Ambiguity resoluton status
    double Ps;                          // Ambiguity resolution success rate [%]
    double ratio;                       // Ambiguity resolution ratio
    int    namb;                        // Number of ambiguities
    int    nfix;                        // Number of fixed ambiguities

    int    stat;                        // Solution quality
    int    nsat;                        // Number of satellites
    double dop[5];                      // Dilution of precision (GDOP/PDOP/HDOP/VDOP/TDOP)

    double lom;                         // Local overall model test value
    double age;                         // Age of differential [s]
} sol_t;

typedef struct sols {                   // Solution data set
    int n, nmax;                        // Number of solutions/allocated memory
    sol_t *sol;                         // Solution data
} sols_t;

typedef struct rcv {                    // Receiver status data
    double  time;                       // Receiver time (GPST) (standard time)
    int     rcv;                        // Receiver index

    double  rr[3];                      // Receiver position (ECEF) [m]
    double  zwd;                        // Zenith wet delay [m]

    double  dtr[NSYS];                  // Receiver clock of each system [m]
    double  cbr[NSYS][NFREQ];           // Receiver code bias of each system [m]
    double  pbr[NSYS][NFREQ];           // Receiver phase bias of each system [m]

    double  rr_fix[3];                  // Receiver position (ECEF) [m] (ambiguity fixed)
    double  zwd_fix;                    // Zenith wet delay [m] (ambiguity fixed)

    double  dtr_fix[NSYS];              // Receiver clock of each system [m] (ambiguity fixed)
    double  cbr_fix[NSYS][NFREQ];       // Receiver code bias of each system [m] (ambiguity fixed)
    double  pbr_fix[NSYS][NFREQ];       // Receiver phase bias of each system [m] (ambiguity fixed)
} rcv_t;

typedef struct rcvs {                   // Receiver status data set
    int n, nmax;                        // Number of receivers/allocated memory
    rcv_t *rcv;                         // Receiver status data
} rcvs_t;

typedef struct sat {                    // Satellite status data
    double  time;                       // Satellite time (GPST) (standard time)
    int     sat;                        // Satellite index
    int     iode;                       // IODE

    double  dts;                        // Satellite clock [m]
    double  cbs[NFREQ];                 // Satellite code bias [m]
    double  pbs[NFREQ];                 // Satellite phase bias [m]

    double  dts_fix;                    // Satellite clock [m] (ambiguity fixed)
    double  cbs_fix[NFREQ];             // Satellite code bias [m] (ambiguity fixed)
    double  pbs_fix[NFREQ];             // Satellite phase bias [m] (ambiguity fixed)
} sat_t;

typedef struct sats {                   // Satellite status data set
    int n, nmax;                        // Number of satellites/allocated memory
    sat_t *sat;                         // Satellite status data
} sats_t;

// =============================================================================
// Function declarations
// =============================================================================

void InitSols(sols_t *sols);
void FreeSols(sols_t *sols);


// =============================================================================
// End of header
// =============================================================================
#ifdef __cplusplus
}
#endif

#endif // SOLUTION_H
