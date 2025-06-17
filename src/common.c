// =============================================================================
// Common functions
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#include "common.h"

#ifndef _WIN32
#include <sys/time.h>                   // for gettimeofday, struct timeval
#endif
#include <time.h>                       // for gmtime, struct tm
#include <math.h>                       // for floor
#include <stdio.h>                      // for sscanf, snprintf
#include <stdlib.h>                     // for malloc, exit

// =============================================================================
// Macros
// =============================================================================
#define CAL_STR0 "0000/00/00 00:00:00.000" // Default calendar string

// =============================================================================
// Global variables
// =============================================================================

// Inverse chi-square table (alpha = 0.001) (n=1~200)
const double CHI2INV[200] = {
    10.83, 13.82, 16.27, 18.47, 20.52, 22.46, 24.32, 26.12, 27.88, 29.59,
    31.26, 32.91, 34.53, 36.12, 37.70, 39.25, 40.79, 42.31, 43.82, 45.31,
    46.80, 48.27, 49.73, 51.18, 52.62, 54.05, 55.48, 56.89, 58.30, 59.70,
    61.10, 62.49, 63.87, 65.25, 66.62, 67.99, 69.35, 70.70, 72.05, 73.40,
    74.74, 76.08, 77.42, 78.75, 80.08, 81.40, 82.72, 84.04, 85.35, 86.66,
    87.97, 89.27, 90.57, 91.87, 93.17, 94.46, 95.75, 97.04, 98.32, 99.61,
   100.89,102.17,103.44,104.72,105.99,107.26,108.53,109.79,111.06,112.32,
   113.58,114.84,116.09,117.35,118.60,119.85,121.10,122.35,123.59,124.84,
   126.08,127.32,128.56,129.80,131.04,132.28,133.51,134.75,135.98,137.21,
   138.44,139.67,140.89,142.12,143.34,144.57,145.79,147.01,148.23,149.45,
   150.67,151.88,153.10,154.31,155.53,156.74,157.95,159.16,160.37,161.58,
   162.79,164.00,165.20,166.41,167.61,168.81,170.02,171.22,172.42,173.62,
   174.82,176.01,177.21,178.41,179.60,180.80,181.99,183.19,184.38,185.57,
   186.76,187.95,189.14,190.33,191.52,192.71,193.89,195.08,196.27,197.45,
   198.64,199.82,201.00,202.18,203.37,204.55,205.73,206.91,208.09,209.26,
   210.44,211.62,212.80,213.97,215.15,216.32,217.50,218.67,219.85,221.02,
   222.19,223.36,224.53,225.71,226.88,228.05,229.21,230.38,231.55,232.72,
   233.89,235.05,236.22,237.39,238.55,239.72,240.88,242.04,243.21,244.37,
   245.53,246.70,247.86,249.02,250.18,251.34,252.50,253.66,254.82,255.98,
   257.13,258.29,259.45,260.61,261.76,262.92,264.08,265.23,266.39,267.54
};

// =============================================================================
// Static type definitions (internal use only)
// =============================================================================

// Leap seconds entry (internal structure)
typedef struct {
    cal_t  cal;                         // Calendar date and time when leap second starts (UTC)
    double leaps;                       // Leap seconds (UTC - GPST)
} leapSec_t;

// =============================================================================
// Static global variables
// =============================================================================

// GLONASS frequency channel number (FCN + 8 : must be larger than 0) (-7 <= FCN <= +6)
static int FCN[NSAT_GLO] = {
    +1, // PRN 1, FCN +1
    -4, // PRN 2, FCN -4
    +5, // PRN 3, FCN +5
    +6, // PRN 4, FCN +6
    +1, // PRN 5, FCN +1
    -4, // PRN 6, FCN -4
    +5, // PRN 7, FCN +5
    +6, // PRN 8, FCN +6
    -2, // PRN 9, FCN -2
    -7, // PRN 10, FCN -7
    +0, // PRN 11, FCN +0
    -1, // PRN 12, FCN -1
    -2, // PRN 13, FCN -2
    -7, // PRN 14, FCN -7
    +0, // PRN 15, FCN +0
    -1, // PRN 16, FCN -1
    +4, // PRN 17, FCN +4
    -3, // PRN 18, FCN -3
    +3, // PRN 19, FCN +3
    +2, // PRN 20, FCN +2
    +4, // PRN 21, FCN +4
    -3, // PRN 22, FCN -3
    +3, // PRN 23, FCN +3
    +2, // PRN 24, FCN +2
};

// WGS84 constants (avoid repeated calculation)
static const double WGS84_E2 = FE_WGS84 * (2.0 - FE_WGS84);  // First eccentricity squared

// Day of year at each month
static const int DOY[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

static const cal_t UNIX0 = { 1970, 1,  1, 0, 0, 0.0 };   // Unix time reference
static const cal_t GPST0 = { 1980, 1,  6, 0, 0, 0.0 };   // GPS time reference
static const cal_t GST0  = { 1999, 8, 22, 0, 0, 0.0 };   // Galileo system time reference
static const cal_t BDT0  = { 2006, 1,  1, 0, 0, 0.0 };   // Beidou time reference

// Leap seconds table [Calendar date when starts (UTC), Leap seconds (UTC - GPST)]
static const leapSec_t LEAP_SECONDS[MAX_LEAPS + 1] = {
    {{2017,1,1,0,0,0.0}, -18.0},
    {{2015,7,1,0,0,0.0}, -17.0},
    {{2012,7,1,0,0,0.0}, -16.0},
    {{2009,1,1,0,0,0.0}, -15.0},
    {{2006,1,1,0,0,0.0}, -14.0},
    {{1999,1,1,0,0,0.0}, -13.0},
    {{1997,7,1,0,0,0.0}, -12.0},
    {{1996,1,1,0,0,0.0}, -11.0},
    {{1994,7,1,0,0,0.0}, -10.0},
    {{1993,7,1,0,0,0.0},  -9.0},
    {{1992,7,1,0,0,0.0},  -8.0},
    {{1991,1,1,0,0,0.0},  -7.0},
    {{1990,1,1,0,0,0.0},  -6.0},
    {{1988,1,1,0,0,0.0},  -5.0},
    {{1985,7,1,0,0,0.0},  -4.0},
    {{1983,7,1,0,0,0.0},  -3.0},
    {{1982,7,1,0,0,0.0},  -2.0},
    {{1981,7,1,0,0,0.0},  -1.0},
    {{   0,0,0,0,0,0.0},   0.0}
};

// Base satellite index for each enabled system (use BASE[sys-1])
static const int BASE[NSYS] = {
#if SYS_GPS
    0,
#endif
#if SYS_GLO
    NSAT_GPS,
#endif
#if SYS_GAL
    NSAT_GPS + NSAT_GLO,
#endif
#if SYS_BDS
    NSAT_GPS + NSAT_GLO + NSAT_GAL,
#endif
#if SYS_QZS
    NSAT_GPS + NSAT_GLO + NSAT_GAL + NSAT_BDS,
#endif
#if SYS_IRN
    NSAT_GPS + NSAT_GLO + NSAT_GAL + NSAT_BDS + NSAT_QZS,
#endif
#if SYS_SBS
    NSAT_GPS + NSAT_GLO + NSAT_GAL + NSAT_BDS + NSAT_QZS + NSAT_IRN
#endif
};

// =============================================================================
// Initialization functions
// =============================================================================

// Initialize navigation data struct
void InitNav(nav_t *nav)
{
    // Initialize broadcast ephemeris data
    for (int i = 0; i < NSAT; i++) {
        nav->ephs[i].n = nav->ephs[i].nmax = 0;
        nav->ephs[i].eph = NULL;
    }

    // Initialize data of antenna parameters
    nav->pcvs.n = nav->pcvs.nmax = 0;
    nav->pcvs.pcv = NULL;

    // Initialize processing options
    nav->opt = (opt_t *)malloc(sizeof(opt_t));
    if (nav->opt == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for processing options\n");
        exit(1);
    }
    SetDefaultOpt(nav->opt);
}

// Free navigation data struct
void FreeNav(nav_t *nav)
{
    // Free broadcast ephemeris data
    for (int i = 0; i < NSAT; i++) {
        if (nav->ephs[i].eph != NULL) {
            free(nav->ephs[i].eph);
            nav->ephs[i].eph = NULL;
            nav->ephs[i].n = nav->ephs[i].nmax = 0;
        }
    }

    // Free antenna parameters data
    if (nav->pcvs.pcv != NULL) {
        free(nav->pcvs.pcv);
        nav->pcvs.pcv = NULL;
        nav->pcvs.n = nav->pcvs.nmax = 0;
    }

    // Free processing options
    if (nav->opt != NULL) {
        free(nav->opt);
    }
}
// =============================================================================
// GLONASS frequency channel number operations functions
// =============================================================================

// Get GLONASS frequency channel number (FCN)
int GetFcn(int prn, int *fcn)
{
    // Check if the PRN is valid
    if (prn <= 0 || prn > MAX_PRN_GLO) return 0;

    // Calculate the frequency channel number
    *fcn = FCN[prn - MIN_PRN_GLO] - 8;

    // Check if the frequency channel number is valid
    if (*fcn < -7 || *fcn > 6) return 0;

    return 1;
}

// Set GLONASS frequency channel number (FCN)
void SetFcn(int prn, int fcn)
{
    // Check if the PRN is valid
    if (prn <= 0 || prn > MAX_PRN_GLO) return;

    // Check if the frequency channel number is valid
    if (fcn < -7 || fcn > 6) return;

    // Set the frequency channel number
    FCN[prn - MIN_PRN_GLO] = fcn + 8;
}

// Set default GLONASS frequency channel number (FCN)
void SetDefaultFcn(void)
{
    SetFcn( 1, +1);      // PRN  1: FCN +1
    SetFcn( 2, -4);      // PRN  2: FCN -4
    SetFcn( 3, +5);      // PRN  3: FCN +5
    SetFcn( 4, +6);      // PRN  4: FCN +6
    SetFcn( 5, +1);      // PRN  5: FCN +1
    SetFcn( 6, -4);      // PRN  6: FCN -4
    SetFcn( 7, +5);      // PRN  7: FCN +5
    SetFcn( 8, +6);      // PRN  8: FCN +6
    SetFcn( 9, -2);      // PRN  9: FCN -2
    SetFcn(10, -7);      // PRN 10: FCN -7
    SetFcn(11, +0);      // PRN 11: FCN +0
    SetFcn(12, -1);      // PRN 12: FCN -1
    SetFcn(13, -2);      // PRN 13: FCN -2
    SetFcn(14, -7);      // PRN 14: FCN -7
    SetFcn(15, +0);      // PRN 15: FCN +0
    SetFcn(16, -1);      // PRN 16: FCN -1
    SetFcn(17, +4);      // PRN 17: FCN +4
    SetFcn(18, -3);      // PRN 18: FCN -3
    SetFcn(19, +3);      // PRN 19: FCN +3
    SetFcn(20, +2);      // PRN 20: FCN +2
    SetFcn(21, +4);      // PRN 21: FCN +4
    SetFcn(22, -3);      // PRN 22: FCN -3
    SetFcn(23, +3);      // PRN 23: FCN +3
    SetFcn(24, +2);      // PRN 24: FCN +2
}

// =============================================================================
// Satellite index conversion functions
// =============================================================================

// Convert satellite PRN and system index to satellite index
int Prn2Sat(int sys, int prn)
{
    if (sys <= 0 || sys > NSYS || prn <= 0) return 0;

    // Get system character to validate and determine PRN range
    char str = Sys2Str(sys);
    if (str == '\0') return 0;  // Invalid or disabled system

    // Use BASE lookup table
    int base = BASE[sys - 1];

    // Validate PRN range and calculate satellite index
    switch (str) {
        case STR_GPS:
            if (prn < MIN_PRN_GPS || prn > MAX_PRN_GPS) return 0;
            return base + (prn - MIN_PRN_GPS + 1);
        case STR_GLO:
            if (prn < MIN_PRN_GLO || prn > MAX_PRN_GLO) return 0;
            return base + (prn - MIN_PRN_GLO + 1);
        case STR_GAL:
            if (prn < MIN_PRN_GAL || prn > MAX_PRN_GAL) return 0;
            return base + (prn - MIN_PRN_GAL + 1);
        case STR_BDS:
            if (prn < MIN_PRN_BDS || prn > MAX_PRN_BDS) return 0;
            return base + (prn - MIN_PRN_BDS + 1);
        case STR_QZS:
            if (prn < MIN_PRN_QZS || prn > MAX_PRN_QZS) return 0;
            return base + (prn - MIN_PRN_QZS + 1);
        case STR_IRN:
            if (prn < MIN_PRN_IRN || prn > MAX_PRN_IRN) return 0;
            return base + (prn - MIN_PRN_IRN + 1);
        case STR_SBS:
            if (prn < MIN_PRN_SBS || prn > MAX_PRN_SBS) return 0;
            return base + (prn - MIN_PRN_SBS + 1);
        default:
            return 0;
    }
}

// Convert satellite index to system index and satellite PRN
int Sat2Prn(int sat, int *prn)
{
    if (sat <= 0 || sat > NSAT) {
        if (prn) *prn = 0;
        return 0;
    }

    // Find system by checking BASE array
    int sys = 1;

#if SYS_GPS
    if (sat <= BASE[sys-1] + NSAT_GPS) {
        if (prn) *prn = MIN_PRN_GPS + sat - BASE[sys-1] - 1;
        return sys;
    }
    sys++;
#endif

#if SYS_GLO
    if (sat <= BASE[sys-1] + NSAT_GLO) {
        if (prn) *prn = MIN_PRN_GLO + sat - BASE[sys-1] - 1;
        return sys;
    }
    sys++;
#endif

#if SYS_GAL
    if (sat <= BASE[sys-1] + NSAT_GAL) {
        if (prn) *prn = MIN_PRN_GAL + sat - BASE[sys-1] - 1;
        return sys;
    }
    sys++;
#endif

#if SYS_BDS
    if (sat <= BASE[sys-1] + NSAT_BDS) {
        if (prn) *prn = MIN_PRN_BDS + sat - BASE[sys-1] - 1;
        return sys;
    }
    sys++;
#endif

#if SYS_QZS
    if (sat <= BASE[sys-1] + NSAT_QZS) {
        if (prn) *prn = MIN_PRN_QZS + sat - BASE[sys-1] - 1;
        return sys;
    }
    sys++;
#endif

#if SYS_IRN
    if (sat <= BASE[sys-1] + NSAT_IRN) {
        if (prn) *prn = MIN_PRN_IRN + sat - BASE[sys-1] - 1;
        return sys;
    }
    sys++;
#endif

#if SYS_SBS
    if (sat <= BASE[sys-1] + NSAT_SBS) {
        if (prn) *prn = MIN_PRN_SBS + sat - BASE[sys-1] - 1;
        return sys;
    }
#endif

    if (prn) *prn = 0;
    return 0;
}

// Convert satellite string (CXX) to satellite index
int Str2Sat(satStr_t satStr)
{
    char sysstr;
    int prn;
    if (sscanf(satStr.str, "%c%d", &sysstr, &prn) != 2) return 0;

    // Get system index from system identifier
    int sys = Str2Sys(sysstr);
    if (sys == 0) return 0;

    switch (sysstr) {
        case STR_GPS: prn += MIN_PRN_GPS - 1; break;    // PRN string number: PRN + MIN_PRN_GPS - 1
        case STR_GLO: prn += MIN_PRN_GLO - 1; break;    // PRN string number: PRN + MIN_PRN_GLO - 1
        case STR_GAL: prn += MIN_PRN_GAL - 1; break;    // PRN string number: PRN + MIN_PRN_GAL - 1
        case STR_BDS: prn += MIN_PRN_BDS - 1; break;    // PRN string number: PRN + MIN_PRN_BDS - 1
        case STR_QZS: prn += MIN_PRN_QZS - 1; break;    // PRN string number: PRN + MIN_PRN_QZS - 1
        case STR_IRN: prn += MIN_PRN_IRN - 1; break;    // PRN string number: PRN + MIN_PRN_IRN - 1
        case STR_SBS: prn += 100; break;                // PRN string number: PRN + 100
        default: return 0;
    }

    // Convert the system index and PRN to satellite index
    return Prn2Sat(sys, prn);
}

// Convert satellite index to satellite string (CXX)
satStr_t Sat2Str(int sat)
{
    // Initialize the satellite string
    satStr_t satStr = {{0}};

    // Get system index and PRN from satellite index
    char sysstr;
    int prn;
    sysstr = Sys2Str(Sat2Prn(sat, &prn));
    if (!sysstr) return satStr;

    // Adjust PRN for each system
    switch (sysstr) {
        case STR_GPS: prn -= MIN_PRN_GPS - 1; break;
        case STR_GLO: prn -= MIN_PRN_GLO - 1; break;
        case STR_GAL: prn -= MIN_PRN_GAL - 1; break;
        case STR_BDS: prn -= MIN_PRN_BDS - 1; break;
        case STR_QZS: prn -= MIN_PRN_QZS - 1; break;
        case STR_IRN: prn -= MIN_PRN_IRN - 1; break;
        case STR_SBS: prn -= 100; break;
        default: return satStr;
    }

    // Convert the system index and PRN to satellite string
    snprintf(satStr.str, SAT_STR_SIZE, "%c%02d", sysstr, prn);

    return satStr;
}

// =============================================================================
// Time operations functions
// =============================================================================

// Convert format of calendar date and time to standard time
double Cal2Time(cal_t cal)
{
    // Check if the calendar date and time is valid
    if (cal.year  < 1970 || cal.year  > 2099 ) return 0.0;
    if (cal.mon   < 1    || cal.mon   > 12   ) return 0.0;
    if (cal.day   < 1    || cal.day   > 31   ) return 0.0;
    if (cal.hour  < 0    || cal.hour  > 23   ) return 0.0;
    if (cal.min   < 0    || cal.min   > 59   ) return 0.0;
    if (cal.sec   < 0.0  || cal.sec   >= 60.0) return 0.0;

    // Check if the year is a leap year
    int leap = (cal.year % 4 == 0 && cal.year % 100 != 0) || (cal.year % 400 == 0);

    // Calculate the day of year
    int doy = cal.day + DOY[cal.mon - 1] + (leap && cal.mon > 2 ? 1 : 0);

    // Calculate the standard time
    int days = (cal.year - 1970) * 365 +        // Days from 1970/01/01 to today
               (cal.year - 1969) / 4 -          // Leap year every 4 years
               (cal.year - 1901) / 100 +        // No leap year every 100 years
               (cal.year - 1601) / 400 +        // Leap year every 400 years
               doy - 1;                         // Days from 01/01 to today

    return days * 86400.0 + cal.hour * 3600.0 + cal.min * 60.0 + cal.sec;
}

// Convert format of standard time to calendar date and time
cal_t Time2Cal(double time)
{
    // Check if the standard time is valid
    if (time <= 0.0) return (cal_t){0};

    // 4-year cycle monthly days (normal year, normal year, normal year, leap year)
    static const int mday[48] = {
        31,28,31,30,31,30,31,31,30,31,30,31,  // Year 1 (normal year)
        31,28,31,30,31,30,31,31,30,31,30,31,  // Year 2 (normal year)
        31,28,31,30,31,30,31,31,30,31,30,31,  // Year 3 (normal year)
        31,29,31,30,31,30,31,31,30,31,30,31   // Year 4 (leap year)
    };

    // Calculate the total days and the seconds in a day
    int totalDays = (int)(time / 86400.0);
    double daySec = time - (double)totalDays * 86400.0;

    // Calculate the day in the 4-year cycle
    int dayInCycle = totalDays % 1461;

    // Find the month (4 years = 48 months)
    int mon;
    for (mon = 0; mon < 48; mon++) {
        if (dayInCycle >= mday[mon]) {
            dayInCycle -= mday[mon];
        } else {
            break;
        }
    }

    // Calculate the year, month, and day
    int year = 1970 + (totalDays / 1461) * 4 + (mon / 12);
    int month = (mon % 12) + 1;
    int day = dayInCycle + 1;

    // Calculate the hour, minute, and second
    int hour = (int)(daySec / 3600.0);
    int minute = (int)((daySec - hour * 3600.0) / 60.0);
    double second = daySec - hour * 3600.0 - minute * 60.0;

    return (cal_t){year, month, day, hour, minute, second};
}

// Convert week and tow in GPS time to standard time format
double Gpst2Time(int week, double tow)
{
    // Check if the GPS time of week is valid
    if (tow < 0.0) return 0.0;

    return Cal2Time(GPST0) + week * 7 * 86400.0 + tow;
}

// Convert standard time to week and tow in GPS time
double Time2Gpst(double time, int *week)
{
    // Check if the standard time is valid
    if (time <= Cal2Time(UNIX0)) return -1.0;

    double t0  = Cal2Time(GPST0);
    double sec = time - t0;
    int    w   = (int)floor(sec / (7 * 86400.0));   // GPS week (floor for negative values)

    if (week) *week = w;

    return sec - w * 7 * 86400.0;             // Time of week in GPS time
}

// Convert week and tow in BDS time to standard time format
double Bdt2Time(int week, double tow)
{
    // Check if the BDS time of week is valid
    if (tow < 0.0) return 0.0;

    return Cal2Time(BDT0) + week * 7 * 86400.0 + tow;
}

// Convert standard time to week and tow in BDS time
double Time2Bdt(double time, int *week)
{
    // Check if the standard time is valid
    if (time <= Cal2Time(UNIX0)) return -1.0;

    double t0  = Cal2Time(BDT0);
    double sec = time - t0;
    int    w   = (int)floor(sec / (7 * 86400.0));   // BDS week (floor for negative values)

    if (week) *week = w;

    return sec - w * 7 * 86400.0;             // Time of week in BDS time
}

// Get current system time in UTC and convert to GPST (standard time format)
double TimeGet(void)
{
    cal_t cal = {0};

#ifdef _WIN32
    SYSTEMTIME utcTime;

    // GetSystemTime() returns UTC time (not local time)
    GetSystemTime(&utcTime);
    cal.year  = (int)utcTime.wYear;
    cal.mon   = (int)utcTime.wMonth;
    cal.day   = (int)utcTime.wDay;
    cal.hour  = (int)utcTime.wHour;
    cal.min   = (int)utcTime.wMinute;
    cal.sec   = utcTime.wSecond + utcTime.wMilliseconds * 1e-3;
#else
    struct timeval tv;
    struct tm *utcTm;

    // Get current time and convert to UTC
    if (gettimeofday(&tv, NULL) != 0) return 0.0;  // Error handling

    utcTm = gmtime(&tv.tv_sec);  // gmtime() returns UTC (not local time)
    if (!utcTm) return 0.0;      // Error handling

    cal.year  = (int)(utcTm->tm_year + 1900);
    cal.mon   = (int)(utcTm->tm_mon + 1);
    cal.day   = (int)(utcTm->tm_mday);
    cal.hour  = (int)(utcTm->tm_hour);
    cal.min   = (int)(utcTm->tm_min);
    cal.sec   = (double)(utcTm->tm_sec) + tv.tv_usec * 1e-6;
#endif

    // Convert UTC to GPST for internal processing
    return Utc2Gpst(Cal2Time(cal));
}

// Convert GPS time to UTC time (standard time)
double Gpst2Utc(double gpst)
{
    // Convert GPST to UTC with leap second correction
    // Search in chronological order (newest to oldest)
    for (int i = 0; LEAP_SECONDS[i].cal.year != 0; i++) {
        double utci = gpst + LEAP_SECONDS[i].leaps;   // Convert GPST to UTC
        double utcs = Cal2Time(LEAP_SECONDS[i].cal);  // Leap second start time (UTC)

        if (utci >= utcs) {
            return utci;  // Apply this leap second correction
        }
    }

    return gpst;  // No leap second correction (before 1981)
}

// Convert UTC time to GPS time (standard time)
double Utc2Gpst(double utc)
{
    // Convert UTC to GPST with leap second correction
    // Search in chronological order (newest to oldest)
    for (int i = 0; LEAP_SECONDS[i].cal.year != 0; i++) {
        double utcs = Cal2Time(LEAP_SECONDS[i].cal);  // Leap second start time (UTC)

        if (utc >= utcs) {
            return utc - LEAP_SECONDS[i].leaps;  // Remove leap second correction
        }
    }

    return utc;  // No leap second correction (before 1981)
}

// Convert GPS time to BDS time (standard time)
double Gpst2Bdt(double gpst)
{
    return gpst - 14.0;
}

// Convert BDS time to GPS time (standard time)
double Bdt2Gpst(double bdt)
{
    return bdt + 14.0;
}

// Convert standard time to day of year
int Time2Doy(double time)
{
    // Check if the standard time is valid
    if (time <= Cal2Time(UNIX0)) return -1;

    cal_t cal = Time2Cal(time);

    cal.mon  = 1;
    cal.day  = 1;
    cal.hour = 0;
    cal.min  = 0;
    cal.sec  = 0.0;

    return (int)((time - Cal2Time(cal)) / 86400.0) + 1;
}

// Convert calendar string to date and time
cal_t Str2Cal(calStr_t calStr)
{
    int year, mon, day, hour, min;
    double sec;

    // Check and parse the calendar string
    if (sscanf(calStr.str, "%4d/%2d/%2d %2d:%2d:%lf", &year, &mon, &day, &hour, &min, &sec) != 6) return (cal_t){0};

    // Check if the calendar date and time is valid
    if (year < 1970 || year > 2099) return (cal_t){0};
    if (mon < 1     || mon > 12   ) return (cal_t){0};
    if (day < 1     || day > 31   ) return (cal_t){0};
    if (hour < 0    || hour > 23  ) return (cal_t){0};
    if (min < 0     || min > 59   ) return (cal_t){0};
    if (sec < 0.0   || sec >= 60.0) return (cal_t){0};

    return (cal_t){year, mon, day, hour, min, sec};
}

// Convert calendar date and time to calendar string
calStr_t Cal2Str(cal_t cal, int dec)
{
    calStr_t calStr = {{CAL_STR0}};

    // Check if the calendar date and time is valid
    if (cal.year < 1970 || cal.year > 2099) return calStr;
    if (cal.mon < 1     || cal.mon > 12   ) return calStr;
    if (cal.day < 1     || cal.day > 31   ) return calStr;
    if (cal.hour < 0    || cal.hour > 23  ) return calStr;
    if (cal.min < 0     || cal.min > 59   ) return calStr;
    if (cal.sec < 0.0   || cal.sec >= 60.0) return calStr;

    // Limit decimal places to valid range (0-3)
    if (dec < 0) dec = 0;
    if (dec > 3) dec = 3;

    // Format the calendar string with specified decimal places
    if (dec == 0) {
        snprintf(calStr.str, CAL_STR_SIZE, "%04d/%02d/%02d %02d:%02d:%02d",
                 cal.year, cal.mon, cal.day, cal.hour, cal.min, (int)cal.sec);
    } else {
        int isec = (int)cal.sec;
        int frac = (int)((cal.sec - isec) * pow(10, dec));
        snprintf(calStr.str, CAL_STR_SIZE, "%04d/%02d/%02d %02d:%02d:%02d.%0*d",
                 cal.year, cal.mon, cal.day, cal.hour, cal.min, isec, dec, frac);
    }

    return calStr;
}

// =============================================================================
// GNSS functions
// =============================================================================

// Transform ECEF coordinate to geodetic coordinate
mat_t *Xyz2Llh(const mat_t *xyz)
{
    // Check if the input matrix is valid
    if (xyz->rows != 1 || xyz->cols != 3) return NULL;

    double p = Norm(xyz);
    double z = MatGetD(xyz, 0, 2);
    double r = RE_WGS84;

    // Initialize latitude
    double L = z / (1 - WGS84_E2);

    for (double L0 = L; fabs(L - L0) >= 1E-4; L0 = L) {

        // Latitude
        double sinphi = L0 / sqrt(SQR(p) + SQR(L0));

        // Rho
        r = RE_WGS84 / sqrt(1.0 - WGS84_E2 * sinphi * sinphi);

        // Re compute zk
        L = z + WGS84_E2 * r * sinphi;
    }

    // Initialize geodetic coordinate
    mat_t *llh = Mat(1, 3, DOUBLE);
    if (!llh) return NULL;

    MatSetD(llh, 0, 0, atan2(L, p));
    MatSetD(llh, 0, 1, atan2(MatGetD(xyz, 0, 1), MatGetD(xyz, 0, 0)));
    MatSetD(llh, 0, 2, sqrt(SQR(L) + SQR(p)) - r);

    if (p <= 1E-12 && MatGetD(xyz, 0, 2) >  0.0) MatSetD(llh, 0, 0,  PI/2);
    if (p <= 1E-12 && MatGetD(xyz, 0, 2) <= 0.0) MatSetD(llh, 0, 0, -PI/2);
    if (p <= 1E-12) MatSetD(llh, 0, 1, 0.0);

    return llh;
}

// Transform geodetic coordinate to ECEF coordinate
mat_t *Llh2Xyz(const mat_t *llh)
{
    // Check if the input matrix is valid
    if (llh->rows != 1 || llh->cols != 3) return NULL;

    double sinlat = sin(MatGetD(llh, 0, 0));
    double coslat = cos(MatGetD(llh, 0, 0));
    double sinlon = sin(MatGetD(llh, 0, 1));
    double coslon = cos(MatGetD(llh, 0, 1));

    double v = RE_WGS84 / sqrt(1.0 - WGS84_E2 * sinlat * sinlat);

    // Initialize ECEF coordinate
    mat_t *xyz = Mat(1, 3, DOUBLE);
    if (!xyz) return NULL;

    MatSetD(xyz, 0, 0, (v + MatGetD(llh, 0, 2)) * coslat * coslon);
    MatSetD(xyz, 0, 1, (v + MatGetD(llh, 0, 2)) * coslat * sinlon);
    MatSetD(xyz, 0, 2, (v * (1.0 - WGS84_E2) + MatGetD(llh, 0, 2)) * sinlat);

    return xyz;
}

// Compute rotation matrix to convert ECEF coordinate to local ENU coordinate
mat_t *Xyz2Rot(const mat_t *xyz)
{
    // Check if the input matrix is valid
    if (xyz->rows != 1 || xyz->cols != 3) return NULL;

    mat_t *llh = Xyz2Llh(xyz);
    if (!llh) return NULL;

    double sinlat = sin(MatGetD(llh, 0, 0));
    double coslat = cos(MatGetD(llh, 0, 0));
    double sinlon = sin(MatGetD(llh, 0, 1));
    double coslon = cos(MatGetD(llh, 0, 1));

    // Initialize rotation matrix
    mat_t *rot = Mat(3, 3, DOUBLE);
    if (!rot) return NULL;

    MatSetD(rot, 0, 0, -sinlon);
    MatSetD(rot, 0, 1, coslon);
    MatSetD(rot, 0, 2, 0.0);

    MatSetD(rot, 1, 0, -sinlat * coslon);
    MatSetD(rot, 1, 1, -sinlat * sinlon);
    MatSetD(rot, 1, 2, coslat);

    MatSetD(rot, 2, 0, coslat * coslon);
    MatSetD(rot, 2, 1, coslat * sinlon);
    MatSetD(rot, 2, 2, sinlat);

    return rot;
}

// Transform ECEF coordinate to local ENU coordinate
mat_t *Xyz2Enu(const mat_t *xyz, const mat_t *org)
{
    // Check if the input matrices are valid
    if (xyz->rows != 1 || xyz->cols != 3) return NULL;
    if (org->rows != 1 || org->cols != 3) return NULL;

    // Flag and matrices
    int info = 1;
    mat_t *dxyz = NULL;
    mat_t *rot  = NULL;
    mat_t *enu  = NULL;

    // Difference between ECEF coordinate and origin position
    dxyz = MatAdd(1.0, xyz, false, -1.0, org, false);
    if (!dxyz) info = 0;

    // Compute rotation matrix
    if (info) {
        rot = Xyz2Rot(org);
        if (!rot) info = 0;
    }

    // Rotate ECEF coordinate to local ENU coordinate (enu^T = dxyz^T * rot^T)
    if (info) {
        enu = MatMul(1.0, dxyz, false, 1, rot, true);
    }

    // Free memory
    FreeMat(dxyz);
    FreeMat(rot);

    return enu;
}

// Transform local ENU coordinate to ECEF coordinate
mat_t *Enu2Xyz(const mat_t *enu, const mat_t *org)
{
    // Check if the input matrices are valid
    if (enu->rows != 1 || enu->cols != 3) return NULL;
    if (org->rows != 1 || org->cols != 3) return NULL;

    // Flag and matrices
    int info = 1;
    mat_t *dxyz = NULL;
    mat_t *rot  = NULL;
    mat_t *xyz  = NULL;

    // Compute rotation matrix
    rot = Xyz2Rot(org);
    if (!rot) info = 0;

    // Rotate local ENU coordinate to ECEF coordinate (dxyz^T = enu^T * rot)
    if (info) {
        dxyz = MatMul(1.0, enu, false, 1.0, rot, false);
        if (!dxyz) info = 0;
    }

    // Add origin position
    if (info) {
        xyz = MatAdd(1.0, dxyz, false, 1.0, org, false);
    }

    // Free memory
    FreeMat(dxyz);
    FreeMat(rot);

    return xyz;
}

// Compute satellite azimuth and elevation angle
mat_t *SatAzEl(const mat_t *rs, const mat_t *rr)
{
    // Check if the input matrices are valid
    if (rs->rows != 1 || rs->cols != 3) return NULL;
    if (rr->rows != 1 || rr->cols != 3) return NULL;
    if (Norm(rs) == 0.0) return NULL;

    // Flag and matrices
    int info = 1;
    mat_t *enu = NULL;
    mat_t *azel = NULL;

    // Initialize azimuth and elevation angle
    azel = Mat(1, 2, DOUBLE);
    if (!azel) info = 0;

    // Check receiver position (special case: origin)
    if (info && Norm(rr) == 0.0) {
        MatSetD(azel, 0, 0, 0.0);    // Azimuth: 0 rad
        MatSetD(azel, 0, 1, PI/2);   // Elevation: 90 degrees
    }
    // Normal case: compute from ENU coordinates
    else if (info) {
        enu = Xyz2Enu(rs, rr);
        if (!enu) info = 0;

        if (info) {
            double e = MatGetD(enu, 0, 0);  // East
            double n = MatGetD(enu, 0, 1);  // North
            double u = MatGetD(enu, 0, 2);  // Up

            // Azimuth and elevation angle [rad]
            double az = atan2(e, n);
            double el = atan2(u, sqrt(SQR(e) + SQR(n)));

            // Normalize azimuth to [0, 2π)
            if (az < 0.0) az += 2.0 * PI;

            MatSetD(azel, 0, 0, az);
            MatSetD(azel, 0, 1, el);
        }
    }

    // Clean up
    FreeMat(enu);

    // Return result or NULL on error
    if (!info) {
        FreeMat(azel);
        return NULL;
    }

    return azel;
}

// Compute geometric distance between satellite and receiver
double GeoDist(const mat_t *rs, const mat_t *rr, mat_t *e)
{
    // Check if the input matrices are valid
    if (rs->rows != 1 || rs->cols != 3) return 0.0;
    if (rr->rows != 1 || rr->cols != 3) return 0.0;
    if (e && (e->rows != 1 || e->cols != 3)) return 0.0;

    // Line of sight vector (rs - rr)
    mat_t *los = MatAdd(1.0, rs, false, -1.0, rr, false);
    if (!los) return 0.0;

    // Euclidean distance
    double r = Norm(los);
    if (r == 0.0) {
        FreeMat(los);
        return 0.0;
    }

    // Compute line of sight unit vector if requested
    if (e) {
        MatSetD(e, 0, 0, MatGetD(los, 0, 0) / r);
        MatSetD(e, 0, 1, MatGetD(los, 0, 1) / r);
        MatSetD(e, 0, 2, MatGetD(los, 0, 2) / r);
    }

    // Free memory
    FreeMat(los);

    // Geometric distance corrected for Sagnac effect
    return r + OMGE_GPS * (MatGetD(rs, 0, 0) * MatGetD(rr, 0, 1) - MatGetD(rs, 0, 1) * MatGetD(rr, 0, 0)) / C_LIGHT;
}

// Compute DOPs (GDOP, PDOP, HDOP, VDOP, TDOP)
mat_t *Dops(const mat_t *azels, double elmask)
{
    // Check if the input matrix is valid (at least 4 satellites)
    if (azels->rows < 4 || azels->cols != 2) return NULL;

    // Check if the elevation mask is valid
    if (elmask < 0.0 || elmask > PI/2) return NULL;

    // Flag and matrices
    int info    = 1;
    mat_t *H    = NULL;
    idx_t *ridx = NULL;
    idx_t *cidx = NULL;
    mat_t *Q    = NULL;
    mat_t *dops = NULL;

    // Make H matrix
    H = Mat(azels->rows, 4, DOUBLE);
    if (!H) info = 0;

    // Make index vector
    ridx = TrueIdx(azels->rows);
    cidx = TrueIdx(4);
    if (!ridx || !cidx) info = 0;

    // Make H matrix
    if (info) {
        for (int i = 0; i < azels->rows; i++) {
            double az = MatGetD(azels, i, 0);
            double el = MatGetD(azels, i, 1);
            double cosel = cos(el);

            // Skip if elevation is less than elevation mask
            if (el <= elmask) {
                IdxSetB(ridx, i, false);
                continue;
            }

            MatSetD(H, i, 0, sin(az) * cosel);
            MatSetD(H, i, 1, cos(az) * cosel);
            MatSetD(H, i, 2, sin(el));
            MatSetD(H, i, 3, 1.0);
        }
    }

    // Compute DOPs
    if (info) {

        // Remove rows with elevation less than elevation mask
        MatLogIdxIn(H, ridx, cidx);

        // Compute covariance matrix
        Q = Mat(H->rows, H->rows, DOUBLE);
        if (!Lsq(H, NULL, NULL, NULL, Q, NULL)) info = 0;

        // Initialize DOPs
        dops = Mat(1, 5, DOUBLE);
        if (!dops) info = 0;

        if (info) {

            // Variances
            double vee = MatGetD(Q, 0, 0);
            double vnn = MatGetD(Q, 1, 1);
            double vuu = MatGetD(Q, 2, 2);
            double vtt = MatGetD(Q, 3, 3);

            // Compute DOPs
            MatSetD(dops, 0, 0, sqrt(vee + vnn + vuu + vtt));
            MatSetD(dops, 0, 1, sqrt(vee + vnn + vuu));
            MatSetD(dops, 0, 2, sqrt(vee + vnn));
            MatSetD(dops, 0, 3, sqrt(vuu));
            MatSetD(dops, 0, 4, sqrt(vtt));
        }
    }

    // Free memory
    FreeMat(H);
    FreeIdx(ridx);
    FreeIdx(cidx);
    FreeMat(Q);

    return dops;
}

// Receiver antenna model
mat_t *RcvAntModel(int sat, const mat_t *azel, int nf, const pcv_t *pcv)
{
    // Check if the input matrices are valid
    if (azel->rows != 1 || azel->cols != 2) return NULL;
    if (nf < 1) return NULL;

    // Check antenna parameters
    if (pcv == NULL) return NULL;

    // Check if the system is valid
    int sys = Sat2Prn(sat, NULL);
    if (sys <= 0 || sys > NSYS) return NULL;

    // Check if the bands are valid
    for (int f = 0; f < nf; f++) {
        int band = Fidx2Band(sys, f + 1);
        if (band <= 0 || band > NBAND) return NULL;
    }

    // Initialize variables
    int gps = Str2Sys(STR_GPS);
    int bds = Str2Sys(STR_BDS);
    int band1 = Str2Band('1');
    int band2 = Str2Band('2');

    // Flag and output matrix
    mat_t *dant = Mat(1, nf, DOUBLE);
    if (!dant) return NULL;

    // Initialize variables
    double e[3], x0[19], off[3], var[19], offc, varc;
    for (int i = 0; i < 19; i++) x0[i] = 5.0 * i;

    // Line of sight unit vector
    double az = MatGetD(azel, 0, 0);
    double el = MatGetD(azel, 0, 1);
    double cosel = cos(el);

    e[0] = sin(az) * cosel;
    e[1] = cos(az) * cosel;
    e[2] = sin(el);

    // Check each frequency band
    mat_t eVec   = {3 , 1, DOUBLE, (void *)e  };
    mat_t x0Vec  = {19, 1, DOUBLE, (void *)x0 };
    mat_t offVec = {3 , 1, DOUBLE, (void *)off};
    mat_t varVec = {19, 1, DOUBLE, (void *)var};
    for (int f = 0; f < nf; f++) {

        // Check if the frequency band is valid (fidx is 1-based)
        int band = Fidx2Band(sys, f + 1);

        // Set antenna phase offset
        for (int i = 0; i < 3; i++) {
            off[i] = pcv->off[sys-1][band-1][i];
        }

        // Check and set antenna parameters
        if (Norm(&offVec) > 0.0) {
            for (int i = 0; i < 19; i++) {
                // Set antenna phase variation
                var[i] = pcv->var[sys-1][band-1][i];
            }
        }
        else if (gps && band == band1) {    // Use GPS L1 antenna parameters if band 1 signals
            for (int i = 0; i < 3 ; i++) off[i] = pcv->off[gps-1][band1-1][i];
            for (int i = 0; i < 19; i++) var[i] = pcv->var[gps-1][band1-1][i];
        }
        else if (gps && sys == bds && band == band2) { // Use GPS L1 antenna parameters if BDS B1
            for (int i = 0; i < 3 ; i++) off[i] = pcv->off[gps-1][band1-1][i];
            for (int i = 0; i < 19; i++) var[i] = pcv->var[gps-1][band1-1][i];
        }
        else if (gps) { // Use GPS L2 antenna parameters for other signals
            for (int i = 0; i < 3 ; i++) off[i] = pcv->off[gps-1][band2-1][i];
            for (int i = 0; i < 19; i++) var[i] = pcv->var[gps-1][band2-1][i];
        }
        else {
            // Skip if antenna parameters are not available
            // It could affect the errors of phase measurements
            continue;
        }

        // Compute antenna phase correction (e^T * off)
        if (!Dot(&eVec, &offVec, &offc)) continue;

        // Phase center variation (zenith angle)
        if (!Interp(&x0Vec, &varVec, 90.0 - el * R2D, &varc)) continue;

        // Compute antenna phase correction [m]
        MatSetD(dant, 0, f, -offc + varc);
    }

    return dant;
}

// Tropospheric delay base mapping function
static double MapF(double el, double a, double b, double c)
{
    double sinel = sin(el);
    return (1.0 + a / (1.0 + b / (1.0 + c))) / (sinel + (a / (sinel + b/(sinel + c))));
}

// Compute tropospheric delay mapping function by Niell mapping function
mat_t *TropoMapF(double time, const mat_t *llh, const mat_t *azel)
{
    // Check if the input matrices are valid
    if (llh->rows != 1 || llh->cols != 3) return NULL;
    if (azel->rows != 1 || azel->cols != 2) return NULL;

    // Check if satellite elevation angle is valid
    double el = MatGetD(azel, 0, 1);
    if (el < 0.0 || el > PI/2) return NULL;

    // Additional safety check for very low elevation angles
    if (el < 1e-6) return NULL;  // Reject extremely low elevation angles

    // Check if time is valid
    if (time < 0.0) return NULL;

    // Constants for Niell mapping function
    const double hgts[]= {2.53E-5, 5.49E-3, 1.14E-3};
    const double lats[]= {15, 30, 45, 60, 75};

    const double coef[][5]={
        { 1.2769934E-3, 1.2683230E-3, 1.2465397E-3, 1.2196049E-3, 1.2045996E-3},
        { 2.9153695E-3, 2.9152299E-3, 2.9288445E-3, 2.9022565E-3, 2.9024912E-3},
        { 62.610505E-3, 62.837393E-3, 63.721774E-3, 63.824265E-3, 64.258455E-3},

        { 0.0000000E-0, 1.2709626E-5, 2.6523662E-5, 3.4000452E-5, 4.1202191E-5},
        { 0.0000000E-0, 2.1414979E-5, 3.0160779E-5, 7.2562722E-5, 11.723375E-5},
        { 0.0000000E-0, 9.0128400E-5, 4.3497037E-5, 84.795348E-5, 170.37206E-5},

        { 5.8021897E-4, 5.6794847E-4, 5.8118019E-4, 5.9727542E-4, 6.1641693E-4},
        { 1.4275268E-3, 1.5138625E-3, 1.4572752E-3, 1.5007428E-3, 1.7599082E-3},
        { 4.3472961E-2, 4.6729510E-2, 4.3908931E-2, 4.4626982E-2, 5.4736038E-2},
    };

    // Latitude [deg] and height
    double lat = MatGetD(llh, 0, 0) * R2D;
    double hgt = MatGetD(llh, 0, 2);

    // Year from doy 28, added half a year for southern latitudes
    double year = (Time2Doy(time) - 28) / 365.25;
    year += (lat < 0.0) ? 0.5 : 0.0;

    // Mapping function
    lat = fabs(lat);
    double cosy = cos(2.0 * PI * year);

    // Create mat_t structures once for efficiency
    const mat_t latVec = {5, 1, DOUBLE, (void *)lats};

    // Interpolate coefficients of hydrostatic
    double ah[3], aw[3], amp;
    for (int i = 0; i < 3; i++) {
        const mat_t avgVec = {5, 1, DOUBLE, (void *)coef[i]};
        const mat_t ampVec = {5, 1, DOUBLE, (void *)coef[i+3]};
        const mat_t wetVec = {5, 1, DOUBLE, (void *)coef[i+6]};

        if (!Interp(&latVec, &avgVec, lat, ah+i)) return NULL;
        if (!Interp(&latVec, &ampVec, lat, &amp)) return NULL;
        if (!Interp(&latVec, &wetVec, lat, aw+i)) return NULL;

        ah[i] = ah[i] - amp * cosy;
    }

    // Ellipsoidal height is used instead of height above sea level
    double dm = (1.0/sin(el) - MapF(el, hgts[0], hgts[1], hgts[2])) * hgt / 1E3;

    // Compute tropospheric delay mapping function
    mat_t *mapf = Mat(1, 2, DOUBLE);
    if (!mapf) return NULL;

    MatSetD(mapf, 0, 0, MapF(el, ah[0], ah[1], ah[2]) + dm);
    MatSetD(mapf, 0, 1, MapF(el, aw[0], aw[1], aw[2]));

    return mapf;
}

// Compute tropospheric delay by standard atmosphere and Saastamoinen model
mat_t *TropoModel(double time, const mat_t *llh, const mat_t *azel, double humi)
{
    // Check if the input matrices are valid
    if (llh->rows != 1 || llh->cols != 3) return NULL;
    if (azel->rows != 1 || azel->cols != 2) return NULL;

    // Check if time is valid
    if (time < 0.0) return NULL;

    // Check elevation angle
    double el = MatGetD(azel, 0, 1);
    if (el < 1E-6 || el > PI/2) return NULL;

    // Check if the humidity is valid
    if (humi < 0.0 || humi > 1.0) return NULL;

    // Initialize output matrix
    mat_t *tropo = Zeros(1, 2, DOUBLE);
    if (!tropo) return NULL;

    // Check position height
    double hgt = MatGetD(llh, 0, 2);
    if (hgt > 1E4 || hgt < -1E2) return tropo;

    // Standard atmosphere
    hgt = hgt < 0.0 ? 0.0 : hgt;

    double pres = 1013.25 * pow(1.0 - 2.2557E-5 * hgt, 5.2568);
    double temp = 15 - 0.0065 * hgt + 273.16;
    double e    = 6.108 * humi * exp((17.15 * temp - 4684.0) / (temp - 38.45));

    // Mapping function
    mat_t *mapf = TropoMapF(time, llh, azel);
    if (!mapf) {
        FreeMat(tropo);
        return NULL;
    }

    // Compute tropospheric delay by standard atmosphere and Saastamoinen model
    double zdry = 0.0022768 * pres / (1.0 - 0.00266 * cos(2.0 * MatGetD(llh, 0, 0)) - 0.00028 * hgt / 1E3);
    double zwet = 0.002277 * (1255.0 / temp + 0.05) * e;

    // Compute tropospheric total delay and variance
    double total = zdry * MatGetD(mapf, 0, 0) + zwet * MatGetD(mapf, 0, 1);
    MatSetD(tropo, 0, 0, total);
    MatSetD(tropo, 0, 1, SQR(STD_SAAS));

    // Free memory
    FreeMat(mapf);

    return tropo;
}

// Compute ionospheric delay by GPS broadcast ionospheric model (Klobuchar)
mat_t *IonoModel(double time, const mat_t *llh, const mat_t *azel, const mat_t *param)
{
    // Check if the input matrices are valid
    if (llh->rows != 1 || llh->cols != 3) return NULL;
    if (azel->rows != 1 || azel->cols != 2) return NULL;

    // Check if time is valid
    if (time < 0.0) return NULL;

    // Check elevation angle
    double el = MatGetD(azel, 0, 1);
    if (el < 1E-6 || el > PI/2) return NULL;

    // Check position height
    double hgt = MatGetD(llh, 0, 2);
    if (hgt < -1E3) return NULL;

    // Default ionosphere model parameters (2004/01/01)
    const double defparam[] = {
        0.1118E-07, -0.7451E-08, -0.5961E-07,  0.1192E-06,
        0.1167E+06, -0.2294E+06, -0.1311E+06,  0.1049E+07,
    };
    const mat_t defparamVec = {1, 8, DOUBLE, (void *)defparam};

    // Use default parameters if input parameters are invalid or zero
    const mat_t *ionparam = param;
    if (!param || param->rows != 1 || param->cols != 8 || Norm(param) == 0.0) {
        ionparam = &defparamVec;
    }

    // Initialize output matrix
    mat_t *iono = Mat(1, 2, DOUBLE);
    if (!iono) return NULL;

    // Extract position and satellite direction
    double lat = MatGetD(llh, 0, 0);  // [rad]
    double lon = MatGetD(llh, 0, 1);  // [rad]
    double az = MatGetD(azel, 0, 0);  // [rad]

    // Earth centered angle (semi-circle)
    double psi = 0.0137 / (el/PI + 0.11) - 0.022;

    // Subionospheric latitude and longitude (semi-circle)
    double phi = lat/PI + psi * cos(az);
    if      (phi >  0.416) phi =  0.416;
    else if (phi < -0.416) phi = -0.416;

    double lam = lon/PI + psi * sin(az) / cos(phi * PI);

    // Geometric latitude (semi-circle)
    phi += 0.064 * cos((lam - 1.617) * PI);

    // Local time
    double tow = Time2Gpst(time, NULL);
    double tt = fmod(43200.0 * lam + tow, 86400.0);

    // Slant factor
    double f = 1.0 + 16.0 * pow(0.53 - el/PI, 3.0);

    // Ionospheric delay calculation
    double amp = MatGetD(ionparam, 0, 0) + phi * (MatGetD(ionparam, 0, 1) +
                 phi * (MatGetD(ionparam, 0, 2) + phi * MatGetD(ionparam, 0, 3)));
    double per = MatGetD(ionparam, 0, 4) + phi * (MatGetD(ionparam, 0, 5) +
                 phi * (MatGetD(ionparam, 0, 6) + phi * MatGetD(ionparam, 0, 7)));

    // Apply constraints
    amp = amp < 0.0     ? 0.0     : amp;
    per = per < 72000.0 ? 72000.0 : per;

    // Phase argument
    double x = 2.0 * PI * (tt - 50400.0) / per;

    // Compute ionospheric delay [m]
    double delay;
    if (fabs(x) < 1.57) {
        delay = C_LIGHT * f * (5E-9 + amp * (1.0 + x * x * (-0.5 + x * x / 24.0)));
    } else {
        delay = C_LIGHT * f * 5E-9;
    }

    // Set output: delay [m] and variance [m^2]
    MatSetD(iono, 0, 0, delay);
    MatSetD(iono, 0, 1, SQR(STD_KLOB_FACTOR * delay));

    return iono;
}

// Compute phase and code measurement error variance
mat_t *MeasVar(int sat, double el, int nf, const opt_t *opt)
{
    // Check if the input matrices are valid
    if (el < 1E-6 || el > PI/2) return NULL;

    // Check if satellite system is valid
    int sys = Sat2Prn(sat, NULL);
    if (sys <= 0 || sys > NSYS) return NULL;

    // Check if frequency index is valid
    if (nf <= 0 || nf > NFREQ) return NULL;

    // Check if processing options are valid
    if (!opt) return NULL;

    // System error factor
    double fact;
    switch (Sys2Str(sys)) {
        case STR_GPS: fact = ERR_FACTOR_GPS; break;
        case STR_GLO: fact = ERR_FACTOR_GLO; break;
        case STR_GAL: fact = ERR_FACTOR_GAL; break;
        case STR_BDS: fact = ERR_FACTOR_BDS; break;
        case STR_QZS: fact = ERR_FACTOR_QZS; break;
        case STR_IRN: fact = ERR_FACTOR_IRN; break;
        case STR_SBS: fact = ERR_FACTOR_SBS; break;
        default: return NULL;
    }

    // Initialize output matrix
    mat_t *var = Mat(2, nf, DOUBLE);
    if (!var) return NULL;

    // Elevation dependent error variance
    for (int f = 0; f < nf; f++) {
        MatSetD(var, 0, f, SQR(fact * opt->err / sin(el)));
        MatSetD(var, 1, f, SQR(opt->errratio) * MatGetD(var, 0, f));
    }

    return var;
}

// =============================================================================
// End of file
// =============================================================================
