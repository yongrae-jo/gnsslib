#ifndef TYPES_H
#define TYPES_H

// Standard library
#include <stddef.h>         // for size_t

// GNSS library
#include "const.h"

// =============================================================================
// 타입 정의(Type Definitions)
// =============================================================================

// -----------------------------------------------------------------------------
// Struct of PCV data
// -----------------------------------------------------------------------------
typedef struct pcv {
    int  sat;                           // Satellite index
    char type  [STA_STR_SIZE];          // Antenna type or SV type
    char serial[STA_STR_SIZE];          // Antenna serial number or sat ID
    double ts;                          // Valid time start
    double te;                          // Valid time end
    double off[NSYS][NBAND][3];         // Phase center offset [m] (sat: xyz, rcv: enu)
    double var[NSYS][NBAND][19];        // Phase center variation [m] (sat: 0,1,...,18, rcv: 90,85,...,0)
} pcv_t;

// -----------------------------------------------------------------------------
// Struct of PCV data set
// -----------------------------------------------------------------------------
typedef struct pcvs {
    int    n, nmax;                    // Number of PCV/allocated memory
    pcv_t  *pcv;                       // PCV data
} pcvs_t;

// -----------------------------------------------------------------------------
// Struct of station parameter data
// -----------------------------------------------------------------------------
typedef struct sta {
    char   name   [STA_STR_SIZE];       // Marker name
    char   marker [STA_STR_SIZE];       // Marker number
    char   antdes [STA_STR_SIZE];       // Antenna descriptor
    char   antsno [STA_STR_SIZE];       // Antenna serial number
    char   rectype[STA_STR_SIZE];       // Receiver type
    char   recsno [STA_STR_SIZE];       // Receiver serial number
    char   recver [STA_STR_SIZE];       // Receiver version
    int    antsetup;                    // Antenna setup ID
    int    itrf;                        // ITRF realization year
    int    deltype;                     // Antenna position delta type (0:enu, 1:xyz)
    double pos[3];                      // Antenna position (ECEF) [m]
    double del[3];                      // Antenna delta position [m]
    int    glo_align;                   // GLONASS code-phase alignment (0:no, 1:yes)
    double glo_bias[4];                 // GLONASS code-phase biases (C1C, C1P, C2C, C2P) [m]
} sta_t;

// -----------------------------------------------------------------------------
// Struct of option data
// -----------------------------------------------------------------------------
typedef struct opt {
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

// -----------------------------------------------------------------------------
// Struct of ephemeris data
// -----------------------------------------------------------------------------
typedef struct eph {
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

// -----------------------------------------------------------------------------
// Struct of ephemeris data set
// -----------------------------------------------------------------------------
typedef struct ephs {
    int    n, nmax;                     // Number of ephemeris/allocated memory
    eph_t  *eph;                        // Ephemeris data
} ephs_t;

// -----------------------------------------------------------------------------
// Struct of navigation data
// -----------------------------------------------------------------------------
typedef struct nav {
    ephs_t    ephs[NSAT];               // Broadcast ephemeris data
    pcvs_t    pcvs;                     // Satellite and receiver antenna PCO and PCV parameters
    sta_t     sta[NRCV];                // Receiver station parameters
    double    iono[NSYS][8];            // Broadcast ionosphere model parameters
    opt_t     *opt;                     // Processing options
} nav_t;

// -----------------------------------------------------------------------------
// Struct of observation data
// -----------------------------------------------------------------------------
typedef struct obs {
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

// -----------------------------------------------------------------------------
// Struct of observation data set
// -----------------------------------------------------------------------------
typedef struct obss {
    int    n, nmax;                    // Number of observations/allocated memory
    obs_t  *obs;                       // Observation data
} obss_t;

// -----------------------------------------------------------------------------
// Struct of solution data
// -----------------------------------------------------------------------------
typedef struct sol {
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

// -----------------------------------------------------------------------------
// Struct of solution data set
// -----------------------------------------------------------------------------
typedef struct sols {
    int n, nmax;                        // Number of solutions/allocated memory
    sol_t *sol;                         // Solution data
} sols_t;

// -----------------------------------------------------------------------------
// Struct of receiver status data
// -----------------------------------------------------------------------------
typedef struct rcv {
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

// -----------------------------------------------------------------------------
// Struct of receiver status data set
// -----------------------------------------------------------------------------
typedef struct rcvs {
    int n, nmax;                        // Number of receivers/allocated memory
    rcv_t *rcv;                         // Receiver status data
} rcvs_t;

// -----------------------------------------------------------------------------
// Struct of satellite status data
// -----------------------------------------------------------------------------
typedef struct sat {
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

// -----------------------------------------------------------------------------
// Struct of satellite status data set
// -----------------------------------------------------------------------------
typedef struct sats {
    int n, nmax;                        // Number of satellites/allocated memory
    sat_t *sat;                         // Satellite status data
} sats_t;

// =============================================================================
// Common utility types
// =============================================================================

// -----------------------------------------------------------------------------
// Struct of satellite string (CXX)
// -----------------------------------------------------------------------------
typedef struct satStr {
    char str[SAT_STR_SIZE];             // Satellite string (CXX)
} satStr_t;

// -----------------------------------------------------------------------------
// Struct of calendar date and time
// -----------------------------------------------------------------------------
typedef struct cal {
    int    year;                        // Year
    int    mon;                         // Month
    int    day;                         // Day
    int    hour;                        // Hour
    int    min;                         // Minute
    double sec;                         // Second
} cal_t;

// -----------------------------------------------------------------------------
// Struct of calendar string (YYYY/MM/DD HH:MM:SS.sss)
// -----------------------------------------------------------------------------
typedef struct calStr {
    char str[CAL_STR_SIZE];             // Calendar string
} calStr_t;

// -----------------------------------------------------------------------------
// Struct of observation code string (LXX)
// -----------------------------------------------------------------------------
typedef struct codeStr {
    char str[CODE_STR_SIZE];            // Observation code string
} codeStr_t;

// =============================================================================
// File management types
// =============================================================================

// -----------------------------------------------------------------------------
// Struct of file name string array
// -----------------------------------------------------------------------------
typedef struct files {
    char **names;                       // Dynamic array of file names
    int  n;                             // Current number of files
    int  nmax;                          // Maximum number of files (allocated)
} files_t;

// -----------------------------------------------------------------------------
// Struct of files
// -----------------------------------------------------------------------------
typedef struct file {
    files_t obsfiles;                   // Observation files
    files_t navfiles;                   // Navigation files
    files_t sp3files;                   // SP3 files
    files_t clkfiles;                   // Clock files
    files_t dcbfiles;                   // DCB files
    files_t atxfiles;                   // Antenna exchange files
} file_t;

// -----------------------------------------------------------------------------
// Line information structure
// -----------------------------------------------------------------------------
typedef struct lineinfo {
    size_t start;                       // Start index of the line
    size_t end;                         // End index of the line
    size_t len;                         // Length of the line
} lineinfo_t;

// -----------------------------------------------------------------------------
// Buffer structure
// -----------------------------------------------------------------------------
typedef struct buffer {
    char        *buff;                  // Full buffer
    lineinfo_t  *lineinfo;              // Line information
    size_t      nline;                  // Number of lines
} buffer_t;

// =============================================================================
// Matrix and vector types
// =============================================================================

// -----------------------------------------------------------------------------
// Matrix data type enumeration
// -----------------------------------------------------------------------------
typedef enum {BOOL, INT, DOUBLE} type_t;

// -----------------------------------------------------------------------------
// Index vector type
// -----------------------------------------------------------------------------
typedef struct idx {
    int n;                              // Number of indices
    type_t type;                        // Index type (BOOL, INT)
    void *idx;                          // Index array
} idx_t;

// -----------------------------------------------------------------------------
// Matrix type
// -----------------------------------------------------------------------------
typedef struct mat {
    int rows, cols;                     // Number of rows and columns
    type_t type;                        // Matrix data type (INT, DOUBLE)
    void *data;                         // Matrix data
} mat_t;

#endif // TYPES_H
