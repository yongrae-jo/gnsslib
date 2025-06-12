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
static int FCN[NSAT_GLO] = {0};

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

    // Format the calendar string
    snprintf(calStr.str, CAL_STR_SIZE, "%04d/%02d/%02d %02d:%02d:%02d.%03d",
             cal.year, cal.mon, cal.day, cal.hour, cal.min, (int)cal.sec, (int)((cal.sec - (int)cal.sec) * 1000));

    return calStr;
}

// =============================================================================
// GNSS functions
// =============================================================================

// Transform ECEF coordinate to geodetic coordinate
mat_t *Xyz2Llh(const mat_t *xyz)
{
    // Check if the input matrix is valid
    if (xyz->rows != 3 || xyz->cols != 1) return NULL;

    // Convert ECEF coordinate to geodetic coordinate (WGS84)




}


// =============================================================================
// End of file
// =============================================================================
