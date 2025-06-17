// =============================================================================
// Satellite ephemeris functions
// =============================================================================

#ifndef EPHEMERIS_H
#define EPHEMERIS_H

#include "const.h"

// =============================================================================
// Type definitions
// =============================================================================

typedef struct eph {                    // Struct of ephemeris data
    int    sat;                         // Satellite index

    int    IODE;                        // IODE or IODnav (G,R,E,C,J,I)
                                        // (R: 0-6 bit of tb field, C: BDS GBAS algorithm)

    int    IODC;                        // IODC (G,J,I)
    int    AODE;                        // AODE (C)
    int    AODC;                        // AODC (C)

    int    sva;                         // URA index (G,R,E,C,J,I,S) (E: SISA index)
    int    svh;                         // SV health (-1:error, 0:healthy, 1:unhealthy)

    int    week;                        // GPS week (no week rollover) (G,E,C,J,I) (C: BDS week)
    double toes;                        // Toe in GPS week (C: BDT week)
    double ttrs;                        // Ttr in GPS week (C: BDT week)

    double af0;                         // SV clock parameters (G,E,C,J,I,S)
    double af1;
    double af2;

    double taun;                        // SV clock parameters (R)
    double gamn;

    double A;                           // SV orbit parameters (G,E,C,J,I)
    double e;
    double i0;
    double OMG0;
    double omg;
    double M0;
    double deln;
    double OMGd;
    double iodt;

    double crc;
    double crs;
    double cuc;
    double cus;
    double cic;
    double cis;

    int    code;                        // Code on L2 (G,J)
    int    flag;                        // L2P data flag (G,J)
    double fit;                         // Fit interval (G,J)

    int    data;                        // Data source (E)

    int    frq;                         // Satellite frequency number (R)
    int    age;                         // Age of operation (R)

    double tgd[2];                      // TGD correction
                                        // (1) GPS TGD
                                        // (1) GAL BGD E5a/E1 (2) BGD E5b/E1
                                        // (1) BDS TGD1 B1/B3 (2) TGD2 B2/B3
                                        // (1) QZS TGD
                                        // (1) IRN TGD

    double pos[3];                      // Satellite position (R,S)
    double vel[3];                      // Satellite velocity (R,S)
    double acc[3];                      // Satellite acceleration (R,S)

    double toe;                         // Time of ephemeris (GPST) (Standard time)
    double toc;                         // Time of clock (GPST) (Standard time)
    double ttr;                         // Time to transmission (GPST) (Standard time)
} eph_t;

typedef struct ephs {                   // Struct of ephemeris data set
    int    n, nmax;                     // Number of ephemeris/allocated memory
    eph_t  *eph;                        // Ephemeris data
} ephs_t;

// =============================================================================
// Macros
// =============================================================================

#define J2_GLO          1.0826257E-3            // 2nd zonal harmonic of geopotential (GLO)

#define SIN_5           -0.08715574274765817    // sin(-5°)
#define COS_5           0.9961946980917455      // cos(-5°)

#define TSTEP           60.0                    // Time step for GLONASS breoadcast ephemeris (s)

#define TOL_KEPLER      1E-13                   // Tolerance for Kepler's equation

// =============================================================================
// Ephemeris data structure functions
// =============================================================================

// -----------------------------------------------------------------------------
// Initialize ephemeris data set structure
//
// args:
//        ephs_t *ephs (I,O) : ephemeris data set structure
//
// return:
//        void         (-)   : no return value
// -----------------------------------------------------------------------------
void InitEphs(ephs_t *ephs);

// -----------------------------------------------------------------------------
// Free ephemeris data set structure
//
// args:
//        ephs_t *ephs (I) : ephemeris data set structure
//
// return:
//        void         (-)   : no return value
// -----------------------------------------------------------------------------
void FreeEphs(ephs_t *ephs);

// -----------------------------------------------------------------------------
// Add ephemeris data to ephemeris data set structure
//
// args:
//        ephs_t *ephs (I,O) : ephemeris data set structure
//  const eph_t  *eph  (I)   : ephemeris data to add
//
// return:
//        int    info  (O)   : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int AddEph(ephs_t *ephs, const eph_t *eph);

// -----------------------------------------------------------------------------
// Sort ephemeris data set by the order of satellite index and time transmission
// (eph.ttr)
//
// args:
//        ephs_t *ephs (I,O) : ephemeris data set structure
//
// return:
//        void         (-)   : no return value
// -----------------------------------------------------------------------------
void SortEphs(ephs_t *ephs);

// =============================================================================
// Broadcast ephemeris functions
// =============================================================================

// -----------------------------------------------------------------------------
// Get broadcast ephemeris type (default: 0)
//
// GPS: 0 - LNAV
// GLO: 0 - LNAV
// GAL: 0 - I/NAV, 1 - F/NAV
// BDS: 0 - D1
// QZS: 0 - LNAV
// IRN: 0 - LNAV
// SBS: 0 - RINEX SBAS ephemeris type
//
// args:
//        int   sys    (I) : system index
//
// return:
//        int   type   (O) : broadcast ephemeris type (if error, return -1)
// -----------------------------------------------------------------------------
int GetEphType(int sys);

// -----------------------------------------------------------------------------
// Set broadcast ephemeris type
//
// GPS: 0 - LNAV
// GLO: 0 - LNAV
// GAL: 0 - I/NAV, 1 - F/NAV
// BDS: 0 - D1
// QZS: 0 - LNAV
// IRN: 0 - LNAV
// SBS: 0 - RINEX SBAS ephemeris type
//
// args:
//        int   sys    (I) : system index
//        int   type   (I) : broadcast ephemeris type
// -----------------------------------------------------------------------------
void SetEphType(int sys, int type);

// -----------------------------------------------------------------------------
// Convert URA error value to index
//
// args:
//        double  err (I) : error value [m]
//
// return:
//        int     ura (O) : URA index (if error, return -1)
// -----------------------------------------------------------------------------
int Ura2Idx(double err);

// -----------------------------------------------------------------------------
// Convert SISA error value to index
//
// args:
//        double  err (I) : error value [m]
//
// return:
//        int    sisa (O) : SISA index (if error, return -1)
// -----------------------------------------------------------------------------
int Sisa2Idx(double err);

// -----------------------------------------------------------------------------
// Convert URA index to error value [m]
//
// args:
//        int     ura  (I) : URA index
//
// return:
//        double  err  (O) : error value [m] (if error, return -1.0)
// -----------------------------------------------------------------------------
double Idx2Ura(int ura);

// -----------------------------------------------------------------------------
// Convert SISA index to error value [m]
//
// args:
//        int    sisa   (I) : SISA index
//
// return:
//        double  err   (O) : error value [m] (if error, return -1.0)
// -----------------------------------------------------------------------------
double Idx2Sisa(int sisa);

// -----------------------------------------------------------------------------
// Test broadcast ephemeris data
//
// args:
//  const eph_t *eph (I) : broadcast ephemeris data
//
// return:
//        int   flag  (O) : test result (1:ok, 0:error)
// -----------------------------------------------------------------------------
int TestEph(const eph_t *eph);

// -----------------------------------------------------------------------------
// Select broadcast ephemeris data
//
// args:
//        double  ephtime (I) : ephemeris time (GPST) (Standard time) [s]
//        int     sat     (I) : satellite index
//  const nav_t   *nav    (I) : navigation data
//        int     iode    (I) : IODE or IODnav (G,R,E,C,J,I)
//                            : (R: 0-6 bit of tb field) (C: BDS GBAS algorithm)
//                            : (-1: no consider IODE for selection)
//
// return:
//        eph_t   *eph    (O) : selected ephemeris data (if error, return NULL)
// -----------------------------------------------------------------------------
eph_t *SelectEph(double ephtime, int sat, const nav_t *nav, int iode);

// -----------------------------------------------------------------------------
// Compute satellite position, velocity, and clock bias/drift using broadcast
// ephemeris data
//
// args:
//        double  ephtime (I) : ephemeris time (GPST) (Standard time) [s]
//        double  time    (I) : satellite emission time (GPST) (Standard time) [s]
//        int     sat     (I) : satellite index
//  const nav_t   *nav    (I) : navigation data
//        int     iode    (I) : IODE or IODnav (G,R,E,C,J,I)
//                            : (R: 0-6 bit of tb field) (C: BDS GBAS algorithm)
//                            : (-1: no consider IODE for selection)
//        mat_t   *rs     (O) : (optional) satellite position/velocity (ECEF) (1x6) [m,m/s]
//        mat_t   *dts    (O) : (optional) satellite clock bias/drift (1x2) [s,s/s]
//        double  *var    (O) : (optional) variance of position and clock bias [m^2]
//        eph_t   *eph    (O) : (optional) used broadcast ephemeris data
//
// return:
//        int     flag    (O) : test result (1:ok, 0:error)
// -----------------------------------------------------------------------------
int SatPosClkBrdc(double ephtime, double time, int sat, const nav_t *nav,
int iode, mat_t *rs, mat_t *dts, double *var, eph_t *eph);


// =============================================================================
// End of header
// =============================================================================
#ifdef __cplusplus
}
#endif

#endif // EPHEMERIS_H
