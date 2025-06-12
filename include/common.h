// =============================================================================
// Common functions header
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#ifndef COMMON_H
#define COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include "const.h"
#include "matrix.h"
#include "obs.h"
#include "option.h"
#include "ephemeris.h"

// =============================================================================
// Macros
// =============================================================================

#define MAX_LEAPS   64                  // Maximum number of leaps table
#define MAX_STR_LEN 64                  // Maximum number of characters in parameter string

#define SAT_STR_SIZE 4                  // Number of characters in satellite string (CXX)
#define CAL_STR_SIZE 24                 // Number of characters in calendar string (YYYY/MM/DD HH:MM:SS.sss)

// =============================================================================
// Type definition
// =============================================================================

typedef struct pcv {                    // Struct of PCV data
    int  sat;                           // Satellite index
    char type[MAX_STR_LEN];             // Antenna type or SV type
    char serial[MAX_STR_LEN];           // Antenna serial number or sat ID
    double ts;                          // Valid time start
    double te;                          // Valid time end
    double off[NSYS][NBAND][3];         // Phase center offset [m] (sat: xyz, rcv: enu)
    double var[NSYS][NBAND][19];        // Phase center variation [m] (sat: 0,1,...,18, rcv: 90,85,...,0)
} pcv_t;

typedef struct pcvs {                  // Struct of PCV data set
    int    n, nmax;                    // Number of PCV/allocated memory
    pcv_t  *pcv;                       // PCV data
} pcvs_t;

typedef struct sta {                    // Struct of station parameter data
    char   name   [MAX_STR_LEN];        // Marker name
    char   marker [MAX_STR_LEN];        // Marker number
    char   antdes [MAX_STR_LEN];        // Antenna descriptor
    char   antsno [MAX_STR_LEN];        // Antenna serial number
    char   rectype[MAX_STR_LEN];        // Receiver type
    char   recsno [MAX_STR_LEN];        // Receiver serial number
    int    antsetup;                    // Antenna setup ID
    int    itrf;                        // ITRF realization year
    int    deltype;                     // Antenna position delta type (0:enu, 1:xyz)
    double pos[3];                      // Antenna position (ECEF) [m]
    double del[3];                      // Antenna delta position [m]
    int    glo_align;                   // GLONASS code-phase alignment (0:no, 1:yes)
    double glo_bias[4];                 // GLONASS code-phase biases (C1C, C1P, C2C, C2P) [m]
} sta_t;

typedef struct nav {                    // Struct of navigation data
    ephs_t    ephs[NSAT];               // Broadcast ephemeris data
    //sp3s_t    sp3s[NSAT];               // SP3 data (TBD)
    //dcbs_t    dcbs[NSAT];               // DCB data (TBD)
    pcvs_t    pcvs;                     // Satellite and receiver antenna PCO and PCV parameters
    sta_t     sta[NRCV];                // Receiver station parameters
    int       iono[NSYS][8];            // Broadcast ionosphere model parameters
    opt_t     *opt;                     // Processing options
} nav_t;

typedef struct satStr {                 // Struct of satellite string (CXX)
    char str[SAT_STR_SIZE];             // Satellite string (CXX)
} satStr_t;

typedef struct cal {                    // Struct of calendar date and time
    int    year;                        // Year
    int    mon;                         // Month
    int    day;                         // Day
    int    hour;                        // Hour
    int    min;                         // Minute
    double sec;                         // Second
} cal_t;

typedef struct calStr {                 // Struct of calendar string (YYYY/MM/DD HH:MM:SS.sss)
    char str[CAL_STR_SIZE];             // Calendar string
} calStr_t;

// =============================================================================
// Global variables
// =============================================================================

// Inverse chi-square table (alpha = 0.001) (n=1~200)
extern const double CHI2INV[200];

// =============================================================================
// Inline functions
// =============================================================================

// Square function for double
static inline double SQR(double x) { return x * x; }

// Convert system index to system string
static inline char Sys2Str(int sys)
{
    int idx = 0;

    if (SYS_GPS && ++idx == sys) return STR_GPS;
    if (SYS_GLO && ++idx == sys) return STR_GLO;
    if (SYS_GAL && ++idx == sys) return STR_GAL;
    if (SYS_BDS && ++idx == sys) return STR_BDS;
    if (SYS_QZS && ++idx == sys) return STR_QZS;
    if (SYS_IRN && ++idx == sys) return STR_IRN;
    if (SYS_SBS && ++idx == sys) return STR_SBS;

    return '\0';
}

// Convert system string to system index
static inline int Str2Sys(const char str)
{
    int idx = 0;

    if (SYS_GPS && ++idx && str == STR_GPS) return idx;
    if (SYS_GLO && ++idx && str == STR_GLO) return idx;
    if (SYS_GAL && ++idx && str == STR_GAL) return idx;
    if (SYS_BDS && ++idx && str == STR_BDS) return idx;
    if (SYS_QZS && ++idx && str == STR_QZS) return idx;
    if (SYS_IRN && ++idx && str == STR_IRN) return idx;
    if (SYS_SBS && ++idx && str == STR_SBS) return idx;

    return 0;
}

// =============================================================================
// Initialization functions
// =============================================================================

// -----------------------------------------------------------------------------
// Initialize navigation data struct
//
// args:
//        nav_t *nav (I,O) : navigation data struct
// -----------------------------------------------------------------------------
void InitNav(nav_t *nav);

// -----------------------------------------------------------------------------
// Free navigation data struct
//
// args:
//        nav_t *nav (I) : navigation data struct
// -----------------------------------------------------------------------------
void FreeNav(nav_t *nav);

// =============================================================================
// GLONASS frequency channel number operations functions
// =============================================================================

// -----------------------------------------------------------------------------
// Get GLONASS frequency channel number (FCN)
//
// args:
//        int   prn     (I) : GLONASS satellite PRN number
//        int  *fcn     (O) : frequency channel number (-7 <= FCN <= +6)
//
// return:
//        int   info    (O) : 1 if successful, 0 if failed
// -----------------------------------------------------------------------------
int GetFcn(int prn, int *fcn);

// -----------------------------------------------------------------------------
// Set GLONASS frequency channel number (FCN)
//
// args:
//        int   prn     (I) : GLONASS satellite PRN number
//        int   fcn     (I) : frequency channel number (-7 <= FCN <= +6)
//
// return:
//        void          (-) : no return value
// -----------------------------------------------------------------------------
void SetFcn(int prn, int fcn);

// -----------------------------------------------------------------------------
// Set default GLONASS frequency channel number (FCN)
//
// args:
//        void          (-) : no input arguments
//
// return:
//        void          (-) : no return value
// -----------------------------------------------------------------------------
void SetDefaultFcn(void);

// =============================================================================
// Satellite index conversion functions
// =============================================================================

// -----------------------------------------------------------------------------
// Convert satellite PRN and system index to satellite index
//
// args:
//        int   sys     (I) : satellite system index
//        int   prn     (I) : satellite PRN number
//
// return:
//        int   sat     (O) : satellite index (0 if error)
// -----------------------------------------------------------------------------
int Prn2Sat(int sys, int prn);

// -----------------------------------------------------------------------------
// Convert satellite index to system index and satellite PRN
//
// args:
//        int   sat     (I) : satellite index
//        int  *prn     (O) : (optional) satellite PRN number (0 if error)
//
// return:
//        int   sys     (O) : satellite system index (0 if error)
// -----------------------------------------------------------------------------
int Sat2Prn(int sat, int *prn);

// -----------------------------------------------------------------------------
// Convert satellite string (CXX) to satellite index
//
// args:
//        satstr_t satstr (I) : satellite string (CXX format)
//
// return:
//        int      sat    (O) : satellite index (0 if error)
// -----------------------------------------------------------------------------
int Str2Sat(satStr_t satStr);

// -----------------------------------------------------------------------------
// Convert satellite index to satellite string (CXX)
//
// args:
//        int   sat     (I) : satellite index
//
// return:
//        satstr_t satstr (O) : satellite string (CXX format, empty if error)
// -----------------------------------------------------------------------------
satStr_t Sat2Str(int sat);

// =============================================================================
// Time operations functions
// =============================================================================

// -----------------------------------------------------------------------------
// Convert format of calendar date and time to standard time
//
// args:
//        cal_t  cal  (I) : calendar date and time
//
// return:
//        double time (O) : standard time
// -----------------------------------------------------------------------------
double Cal2Time(cal_t cal);

// -----------------------------------------------------------------------------
// Convert format of standard time to calendar date and time
//
// args:
//        double time (I) : standard time
//
// return:
//        cal_t cal   (O) : calendar date and time
// -----------------------------------------------------------------------------
cal_t Time2Cal(double time);

// -----------------------------------------------------------------------------
// Convert week and tow in GPS time to standard time format
//
// args:
//        int    week (I) : GPS week
//        double tow  (I) : GPS time of week
//
// return:
//        double time (O) : standard time (GPST) (if error, return 0.0)
// -----------------------------------------------------------------------------
double Gpst2Time(int week, double tow);

// -----------------------------------------------------------------------------
// Convert standard time to week and tow in GPS time
//
// args:
//        double time  (I) : standard time (GPST)
//        int    *week (O) : (optional) GPS week
//
// return:
//        double tow   (O) : time of week in GPS time (if tow < 0.0, error)
// -----------------------------------------------------------------------------
double Time2Gpst(double time, int *week);

// -----------------------------------------------------------------------------
// Convert week and tow in BDS time to standard time format
//
// args:
//        int    week (I) : BDS week
//        double tow  (I) : BDS time of week
//
// return:
//        double time (O) : standard time (BDS) (if error, return 0.0)
// -----------------------------------------------------------------------------
double Bdt2Time(int week, double tow);

// -----------------------------------------------------------------------------
// Convert standard time to week and tow in BDS time
//
// args:
//        double time  (I) : standard time (BDS)
//        int    *week (O) : (optional) BDS week
//
// return:
//        double time  (O) : time of week in BDS time (if error, return -1.0)
// -----------------------------------------------------------------------------
double Time2Bdt(double time, int *week);

// -----------------------------------------------------------------------------
// Get current system time in UTC and convert to GPST (standard time format)
//
// return:
//        double time (O) : standard time (GPST) - current UTC converted to GPST
// -----------------------------------------------------------------------------
double TimeGet(void);

// -----------------------------------------------------------------------------
// Convert GPS time to UTC time (standard time)
//
// args:
//        double gpst (I) : standard time (GPST)
//
// return:
//        double utc  (O) : standard time (UTC)
// -----------------------------------------------------------------------------
double Gpst2Utc(double time);

// -----------------------------------------------------------------------------
// Convert UTC time to GPS time (standard time)
//
// args:
//        double utc  (I) : standard time (UTC)
//
// return:
//        double gpst (O) : standard time (GPST)
// -----------------------------------------------------------------------------
double Utc2Gpst(double time);

// -----------------------------------------------------------------------------
// Convert GPS time to BDS time (standard time)
//
// args:
//        double gpst (I) : standard time (GPST)
//
// return:
//        double bdt  (O) : standard time (BDS)
// -----------------------------------------------------------------------------
double Gpst2Bdt(double time);

// -----------------------------------------------------------------------------
// Convert BDS time to GPS time (standard time)
//
// args:
//        double bdt  (I) : standard time (BDS)
//
// return:
//        double gpst (O) : standard time (GPST)
// -----------------------------------------------------------------------------
double Bdt2Gpst(double time);

// -----------------------------------------------------------------------------
// Convert standard time to day of year
//
// args:
//        double time (I) : standard time
//
// return:
//        int    doy  (O) : day of year (if error, return -1)
// -----------------------------------------------------------------------------
int Time2Doy(double time);

// -----------------------------------------------------------------------------
// Convert calendar string to date and time
//
// args:
//        calStr_t calStr (I) : calendar string
//
// return:
//        cal_t cal       (O) : calendar date and time
// -----------------------------------------------------------------------------
cal_t Str2Cal(calStr_t calStr);

// -----------------------------------------------------------------------------
// Convert date and time to calendar string
//
// args:
//        cal_t cal        (I) : calendar date and time
//        int   dec        (I) : decimal point
//
// return:
//        calStr_t calStr (O) : calendar string
// -----------------------------------------------------------------------------
calStr_t Cal2Str(cal_t cal, int dec);

// =============================================================================
// GNSS functions
// =============================================================================

// -----------------------------------------------------------------------------
// Transform ECEF coordinate to geodetic position
//
// args:
//  const mat_t *xyz (I) : ECEF coordinate (1 x 3) [m]
//
// return:
//        mat_t *llh (O) : geodetic position (1 x 3) [rad,rad,m] (if error, return NULL)
// -----------------------------------------------------------------------------
mat_t *Xyz2Llh(const mat_t *xyz);

// -----------------------------------------------------------------------------
// Transform geodetic position to ECEF coordinate
//
// args:
//        mat_t *llh (I) : geodetic position (1 x 3) [rad,rad,m]
//
// return:
//        mat_t *xyz (O) : ECEF coordinate (1 x 3) [m] (if error, return NULL)
// -----------------------------------------------------------------------------
mat_t *Llh2Xyz(const mat_t *llh);

// -----------------------------------------------------------------------------
// Compute rotation matrix to convert ECEF coordinate to local ENU coordinate
//
// args:
//        mat_t *xyz (I) : ECEF coordinate (1 x 3) [m]
//
// return:
//        mat_t *rot (O) : rotation matrix (3 x 3) (if error, return NULL)
// -----------------------------------------------------------------------------
mat_t *Xyz2Rot(const mat_t *xyz);

// -----------------------------------------------------------------------------
// Transform ECEF coordinate to local ENU coordinate
//
// args:
//        mat_t *xyz (I) : ECEF coordinate (1 x 3) [m]
//        mat_t *org (I) : ECEF origin coordinate (1 x 3) [m]
//
// return:
//        mat_t *enu (O) : local ENU coordinate (1 x 3) [m] (if error, return NULL)
// -----------------------------------------------------------------------------
mat_t *Xyz2Enu(const mat_t *xyz, const mat_t *org);

// -----------------------------------------------------------------------------
// Transform local ENU coordinate to ECEF coordinate
//
// args:
//        mat_t *enu (I) : local ENU coordinate (1 x 3) [m]
//        mat_t *org (I) : ECEF origin coordinate (1 x 3) [m]
//
// return:
//        mat_t *xyz (O) : ECEF coordinate (1 x 3) [m] (if error, return NULL)
// -----------------------------------------------------------------------------
mat_t *Enu2Xyz(const mat_t *enu, const mat_t *org);

// -----------------------------------------------------------------------------
// Compute satellite azimuth and elevation angle
//
// args:
//        mat_t *rs   (I) : satellite position (1 x 3) [m]
//        mat_t *rr   (I) : receiver position (1 x 3) [m]
//
// return:
//        mat_t *azel (O) : azimuth and elevation angle (1 x 2) [rad,rad]
//                        : (if all(rr) == 0.0, return [0.0, pi/2])
//                        : (if error, return NULL)
// -----------------------------------------------------------------------------
mat_t *SatAzEl(const mat_t *rs, const mat_t *rr);

// -----------------------------------------------------------------------------
// Compute geometric distance and line of sight unit vector between satellite
// and receiver
//
// args:
//        mat_t *rs   (I) : satellite position (1 x 3) [m]
//        mat_t *rr   (I) : receiver position (1 x 3) [m]
//        mat_t  *e   (O) : (optional) line of sight unit vector (1 x 3) (ECEF)
//
// return:
//        double dist (O) : geometric distance [m] (if error, return 0.0)
// -----------------------------------------------------------------------------
double GeoDist(const mat_t *rs, const mat_t *rr, mat_t *e);

// -----------------------------------------------------------------------------
// Compute DOPs (GDOP, PDOP, HDOP, VDOP, TDOP)
//
// args:
//  const mat_t  *azels (I) : azimuth and elevation angles (n x 2) [rad,rad]
//        double elmask (I) : elevation mask angle [rad]
//
// return:
//        mat_t  *dops  (O) : DOPs (1 x 5) [GDOP, PDOP, HDOP, VDOP, TDOP]
// -----------------------------------------------------------------------------
mat_t *Dops(const mat_t *azels, double elmask);

// -----------------------------------------------------------------------------
// Receiver antenna model
//
// args:
//        int   sat   (I) : satellite index
//  const mat_t *azel (I) : azimuth and elevation angle (1 x 2) [rad,rad]
//        int   nf    (I) : number of frequency
//  const pcv_t *pcv  (I) : PCV data
//
// return:
//        mat_t *dant (O) : antenna correction (1 x nf) [m] (if error, return NULL)
// -----------------------------------------------------------------------------
mat_t *RcvAntModel(int sat, const mat_t *azel, int nf, const pcv_t *pcv);

// -----------------------------------------------------------------------------
// Compute tropospheric delay mapping function by Niell mapping function
//
// args:
//        double time  (I) : standard time (GPST)
//  const mat_t *llh   (I) : geodetic position (1 x 3) [rad,rad,m]
//  const mat_t *azel  (I) : azimuth and elevation angle (1 x 2) [rad,rad]
//
// return:
//        mat_t *mapf (O) : dry and wet mapping function (1 x 2) [dry, wet]
//                        : (if error, return NULL)
// -----------------------------------------------------------------------------
mat_t *TropoMapF(double time, const mat_t *llh, const mat_t *azel);

// -----------------------------------------------------------------------------
// Compute tropospheric delay by standard atmosphere and Saastamoinen model
//
// args:
//        double time (I) : standard time (GPST)
//  const mat_t *llh  (I) : geodetic position (1 x 3) [rad,rad,m]
//  const mat_t *azel (I) : azimuth and elevation angle (1 x 2) [rad,rad]
//        double humi (I) : relative humidity [%] (0.0 ~ 1.0)
//
// return:
//        mat_t *trop (O) : tropospheric delay and variance (1 x 2) [m, m^2]
//                        : (if error, return NULL)
// -----------------------------------------------------------------------------
mat_t *TropoModel(double time, const mat_t *llh, const mat_t *azel, double humi);

// -----------------------------------------------------------------------------
// Compute ionospheric delay by GPS broadcast ionospheric model (Klobuchar)
//
// args:
//        double time  (I) : standard time (GPST)
//  const mat_t *llh   (I) : geodetic position (1 x 3) [rad,rad,m]
//  const mat_t *azel  (I) : azimuth and elevation angle (1 x 2) [rad,rad]
//  const mat_t *param (I) : GPS broadcast ionospheric model parameters (1 x 8)
//
// return:
//        mat_t *iono  (O) : ionospheric delay and variance (1 x 2) [m, m^2]
//                         : (if error, return NULL)
// -----------------------------------------------------------------------------
mat_t *IonoModel(double time, const mat_t *llh, const mat_t *azel, const mat_t *param);

// -----------------------------------------------------------------------------
// Compute phase and code measurement error variance
//
// args:
//        int   sat   (I) : satellite index
//        double el   (I) : elevation angle [rad]
//        int   nf    (I) : number of frequency
//  const opt_t *opt  (I) : processing options
//
// return:
//        mat_t *var  (O) : phase and code measurement error variance (2 x nf)
//                        : (if error, return NULL)
// -----------------------------------------------------------------------------
mat_t *MeasVar(int sat, double el, int nf, const opt_t *opt);

// =============================================================================
// End of header
// =============================================================================
#ifdef __cplusplus
}
#endif

#endif // COMMON_H
