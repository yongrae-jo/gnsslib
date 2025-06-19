// =============================================================================
// RINEX file functions
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

// Standard library
#include <math.h>                       // for round, floor, fmod
#include <string.h>                     // for strlen, strcpy, strncmp
#include <stdlib.h>                     // for free
#include <stdio.h>                      // for sscanf, sprintf

// GNSS library
#include "rinex.h"
#include "files.h"                      // for buffer_t, GetLine
#include "common.h"                     // for Sys2Str
#include "obs.h"                        // for Str2Code, Code2Fidx, AddObs, SortObss
#include "ephemeris.h"                  // for AddEph, SortEph

// =============================================================================
// Macros
// =============================================================================

#define MAX_OBSTYPE 32                   // Number of observation types

// =============================================================================
// Static type definitions
// =============================================================================

typedef struct rnxObsHeader {               // RINEX observation header structure
    double ver;                             // RINEX version number
    char   sys;                             // System string
    char   obsType[NSYS][MAX_OBSTYPE][4];   // Observation types
    int    nObsType[NSYS];                  // Number of observation types per system
    sta_t  sta;                             // Station information
} rnxObsHeader_t;

typedef struct rnxObsBody {                 // RINEX observation body structure
    cal_t    cal;                           // Observation epoch time
    satStr_t satStr;                        // Satellite ID string
    double   obs[MAX_OBSTYPE];              // Observation data
    int      lli[MAX_OBSTYPE];              // Loss of lock indicator
} rnxObsBody_t;

typedef struct rnxObs {                     // RINEX observation structure
    rnxObsHeader_t header;                  // RINEX observation header
    rnxObsBody_t   *body;                   // RINEX observation data body
    int            n;                       // Number of observation data
} rnxObs_t;

typedef struct rnxNavHeader {               // RINEX navigation header structure
    double ver;                             // RINEX version number
    char   sys;                             // System string
    double iono[NSYS][8];                   // Broadcast ionospheric parameters
} rnxNavHeader_t;

typedef struct rnxNavBody {                 // RINEX navigation body structure
    cal_t    cal;                           // Time of clock (Toc)
                                            // BeiDou: BDT
                                            // GLONASS: UTC
                                            // Others: GPST

    satStr_t satStr;                        // Satellite ID string
    double   clock[3];                      // Clock parameters
    double   orbit[28];                     // Orbit parameters
} rnxNavBody_t;

typedef struct rnxNav {                     // RINEX navigation structure
    rnxNavHeader_t header;                  // RINEX navigation header
    rnxNavBody_t   *body;                   // RINEX navigation data body
    int            n;                       // Number of navigation data
} rnxNav_t;

// =============================================================================
// Static functions
// =============================================================================

// Convert code from type2 to type3 (1: success, 0: failure)
static int ConvCode(const char *type2, double ver, char sys, char *type3)
{
    // Check if the parameters are valid
    if (!type2 || !type3 || strlen(type2) != 2 || sys == '\0') return 0;

    // Initialize type3
    strcpy(type3, "   ");

    if (!strcmp(type2, "P1")) {
        if (sys == 'G')
            strcpy(type3, "C1W");
        else if (sys == 'R')
            strcpy(type3, "C1P");
    }
    else if (!strcmp(type2, "P2")) {
        if (sys == 'G')
            strcpy(type3, "C2W");
        else if (sys == 'R')
            strcpy(type3, "C2P");
    }
    else if (!strcmp(type2, "C1")) {
        if (ver < 2.12) {
            if (sys == 'G' || sys == 'R' || sys == 'J' || sys == 'S')
                strcpy(type3, "C1C");
            else if (sys == 'E')
                strcpy(type3, "C1X");
        }
    }
    else if (!strcmp(type2, "C2")) {
        if (sys == 'G') {
            if (ver >= 2.12)
                strcpy(type3, "C2W");
            else
                strcpy(type3, "C2X");
        }
        else if (sys == 'R')
            strcpy(type3, "C2C");
        else if (sys == 'J')
            strcpy(type3, "C2X");
        else if (sys == 'C')
            strcpy(type3, "C2I");
    }
    else if (ver >= 2.12) {
        if (type2[1] == 'A') {
            if (sys == 'G' || sys == 'R' || sys == 'J' || sys == 'S')
                sprintf(type3, "%c1C", type2[0]);
        }
        else if (type2[1] == 'B') {
            if (sys == 'G' || sys == 'J')
                sprintf(type3, "%c1X", type2[0]);
        }
        else if (type2[1] == 'C') {
            if (sys == 'G' || sys == 'J')
                sprintf(type3, "%c2X", type2[0]);
        }
        else if (type2[1] == 'D') {
            if (sys == 'R')
                sprintf(type3, "%c2C", type2[0]);
        }
        else if (type2[1] == '1') {
            if (sys == 'G')
                sprintf(type3, "%c1W", type2[0]);
            else if (sys == 'R')
                sprintf(type3, "%c1P", type2[0]);
            else if (sys == 'E')
                sprintf(type3, "%c1X", type2[0]);
            else if (sys == 'C')
                sprintf(type3, "%c2I", type2[0]);
        }
    }
    else { // ver < 2.12
        if (type2[1] == '1') {
            if (sys == 'G' || sys == 'R' || sys == 'J' || sys == 'S')
                sprintf(type3, "%c1C", type2[0]);
            else if (sys == 'E')
                sprintf(type3, "%c1X", type2[0]);
        }
        else if (type2[1] == '2') {
            if (sys == 'G')
                sprintf(type3, "%c2W", type2[0]);
            else if (sys == 'R')
                sprintf(type3, "%c2P", type2[0]);
            else if (sys == 'J')
                sprintf(type3, "%c2X", type2[0]);
            else if (sys == 'C')
                sprintf(type3, "%c2I", type2[0]);
        }
        else if (type2[1] == '5') {
            if (sys == 'G' || sys == 'E' || sys == 'J' || sys == 'S')
                sprintf(type3, "%c5X", type2[0]);
        }
        else if (type2[1] == '6') {
            if (sys == 'E' || sys == 'J')
                sprintf(type3, "%c6X", type2[0]);
            else if (sys == 'C')
                sprintf(type3, "%c6I", type2[0]);
        }
        else if (type2[1] == '7') {
            if (sys == 'E' || sys == 'J')
                sprintf(type3, "%c7X", type2[0]);
            else if (sys == 'C')
                sprintf(type3, "%c7I", type2[0]);
        }
        else if (type2[1] == '8') {
            if (sys == 'E')
                sprintf(type3, "%c8X", type2[0]);
        }
    }

    // Return 1 if conversion was successful
    return strcmp(type3, "   ") != 0;
}

// Check if line contains substring at specific position (safe for non-null-terminated strings)
static int LineContains(const char *line, int len, const char *substr, int pos)
{
    if (!line || !substr || pos < 0 || len <= 0) return 0;

    int sublen = strlen(substr);
    if (pos + sublen > len) return 0;

    return strncmp(line + pos, substr, sublen) == 0;
}

// Remove trailing spaces from string
static void Deblank(char *str, const char *line, int maxlen)
{
    if (!str || !line || maxlen <= 0) return;

    // Copy string with maximum length
    strncpy(str, line, maxlen);
    str[maxlen] = '\0';  // Ensure null termination

    // Remove trailing spaces
    for (int i = maxlen - 1; i >= 0 && str[i] == ' '; i--) {
        str[i] = '\0';
    }
}

// Initialize RINEX observation structure (1: success, 0: failure)
static int InitRnxObs(rnxObs_t *rnxObs)
{
    if (!rnxObs) return 0;

    // Initialize RINEX observation header
    rnxObs->header.ver = 0.0;
    rnxObs->header.sys = '\0';

    for (int i = 0; i < NSYS; i++) {
        rnxObs->header.nObsType[i] = 0;
        for (int j = 0; j < MAX_OBSTYPE; j++) {
            memset(rnxObs->header.obsType[i][j], 0, sizeof(rnxObs->header.obsType[i][j]));
        }
    }

    // Initialize station information
    memset(&rnxObs->header.sta, 0, sizeof(sta_t));

    // Initialize RINEX observation data body
    rnxObs->body = NULL;
    rnxObs->n = 0;

    return 1;
}

// Free RINEX observation structure
static void FreeRnxObs(rnxObs_t *rnxObs)
{
    if (!rnxObs) return;

    if (rnxObs->body) {
        free(rnxObs->body);
        rnxObs->body = NULL;
    }
    rnxObs->n = 0;
}

// Initialize RINEX navigation structure (1: success, 0: failure)
static int InitRnxNav(rnxNav_t *rnxNav)
{
    if (!rnxNav) return 0;

    // Initialize RINEX navigation header
    rnxNav->header.ver = 0.0;
    rnxNav->header.sys = '\0';

    for (int i = 0; i < NSYS; i++) {
        for (int j = 0; j < 8; j++) {
            rnxNav->header.iono[i][j] = 0.0;
        }
    }

    // Initialize RINEX navigation data body
    rnxNav->body = NULL;
    rnxNav->n = 0;

    return 1;
}

// Free RINEX navigation structure
static void FreeRnxNav(rnxNav_t *rnxNav)
{
    if (!rnxNav) return;

    if (rnxNav->body) {
        free(rnxNav->body);
        rnxNav->body = NULL;
    }
    rnxNav->n = 0;
}

// Read RINEX observation file header (1: success, 0: failure)
static int ReadRnxObsHeader(const buffer_t *buffer, rnxObs_t *rnxObs)
{
    if (!buffer || !rnxObs) return 0;

    char *line, type2[3], type3[4];
    int len, endFlag = 0, ntype;
    size_t l = 0;
    double ver, pos[3], hen[3];

    for (l = 0; l < buffer->nline && !endFlag; l++) {

        // Get line from buffer
        if (!(line = GetLine(buffer, l, &len))) continue;

        // Parse header
        if (LineContains(line, len, "RINEX VERSION / TYPE", 60)) {

            // Set version and system
            if (sscanf(line, "%lf", &ver) != 1) return 0;
            rnxObs->header.ver = ver;
            rnxObs->header.sys = line[40];
        }
        else if (LineContains(line, len, "MARKER NAME", 60)) {

            // Set marker name
            Deblank(rnxObs->header.sta.name, line, 20);
        }
        else if (LineContains(line, len, "REC # / TYPE / VERS", 60)) {

            // Set receiver info
            Deblank(rnxObs->header.sta.recsno , line, 20);
            Deblank(rnxObs->header.sta.rectype, line + 20, 20);
            Deblank(rnxObs->header.sta.recver , line + 40, 20);
        }
        else if (LineContains(line, len, "ANT # / TYPE", 60)) {

            // Set antenna info
            Deblank(rnxObs->header.sta.antsno, line, 20);
            Deblank(rnxObs->header.sta.antdes, line + 20, 20);
        }
        else if (LineContains(line, len, "APPROX POSITION XYZ", 60)) {

            // Set approximate position
            if (sscanf(line, "%lf %lf %lf", pos, pos + 1, pos + 2) == 3) {
                rnxObs->header.sta.pos[0] = pos[0];
                rnxObs->header.sta.pos[1] = pos[1];
                rnxObs->header.sta.pos[2] = pos[2];
            }
        }
        else if (LineContains(line, len, "ANTENNA: DELTA H/E/N", 60)) {

            // Set antenna delta
            if (sscanf(line, "%lf %lf %lf", hen, hen + 1, hen + 2) == 3) {
                rnxObs->header.sta.del[0] = hen[1]; // East
                rnxObs->header.sta.del[1] = hen[2]; // North
                rnxObs->header.sta.del[2] = hen[0]; // Up
            }
        }
        else if (LineContains(line, len, "# / TYPES OF OBSERV", 60)) {

            // Set number of observation types
            if (sscanf(line, "%d", &ntype) != 1 || ntype <= 0) continue;
            if (ntype > MAX_OBSTYPE) ntype = MAX_OBSTYPE;

            // Set number of observation types for all systems
            for (int c = 0; c < NSYS; c++) rnxObs->header.nObsType[c] = ntype;

            // Number of lines for observation types
            int nlines = (ntype + 8) / 9; // Ceiling division

            for (int i = 0; i < nlines; i++) {

                // Number of observation types per line
                int ntypei = (i == nlines - 1) ? (ntype - 9 * i) : 9;

                // Get line from buffer
                char *ptr = GetLine(buffer, l + i, &len);
                if (!ptr || len < 10 + ntypei * 6) return 0;

                for (int j = 0; j < ntypei; j++) {

                    // Set type2
                    strncpy(type2, ptr + 10 + j * 6, 2);
                    type2[2] = '\0';

                    // Index of obstype
                    int oidx = i * 9 + j;

                    for (int c = 0; c < NSYS; c++) {

                        // Convert type2 to type3
                        if (ConvCode(type2, rnxObs->header.ver, Sys2Str(c+1), type3)) {

                            // Set observation type
                            strcpy(rnxObs->header.obsType[c][oidx], type3);
                        }
                    }
                }
            }

            l = l + nlines - 1; // Skip to next header
        }
        else if (LineContains(line, len, "SYS / # / OBS TYPES", 60)) {

            // Get system character
            char sys = line[0];
            int sysidx = Str2Sys(sys);

            // Set number of observation types
            if (sscanf(line + 3, "%d", &ntype) != 1 || ntype <= 0 || sysidx <= 0) continue;
            if (ntype > MAX_OBSTYPE) ntype = MAX_OBSTYPE;

            // Set number of observation types for this system
            rnxObs->header.nObsType[sysidx - 1] = ntype;

            // Number of lines for observation types (13 types per line for v3)
            int nlines = (ntype + 12) / 13; // Ceiling division

            for (int i = 0; i < nlines; i++) {

                // Number of observation types per line
                int ntypei = (i == nlines - 1) ? (ntype - 13 * i) : 13;

                // Get line from buffer
                char *ptr = GetLine(buffer, l + i, &len);
                if (!ptr || len < 7 + ntypei * 4) return 0;

                for (int j = 0; j < ntypei; j++) {

                    // Index of obstype
                    int oidx = i * 13 + j;

                    // Set observation type (4 characters starting from position 7 + j*4)
                    strncpy(rnxObs->header.obsType[sysidx - 1][oidx], ptr + 7 + j * 4, 3);
                    rnxObs->header.obsType[sysidx - 1][oidx][3] = '\0';

                    // Change BDS B1 to B2 (RINEX version 3.02 only)
                    if (rnxObs->header.ver == 3.02 && sys == 'C') {
                        if (rnxObs->header.obsType[sysidx - 1][oidx][1] == '1') {
                            rnxObs->header.obsType[sysidx - 1][oidx][1] = '2';
                        }
                    }
                }
            }

            l = l + nlines - 1; // Skip to next header
        }
        else if (LineContains(line, len, "END OF HEADER", 60)) {
            endFlag = 1;
        }
    }

    return endFlag;
}

// Read RINEX v2 observation file body
static int ReadRnxObsBodyV2(const buffer_t *buffer, rnxObs_t *rnxObs, size_t startLine)
{
    if (!buffer || !rnxObs) return 0;

    int len, nepoch = 0, total = 0;
    size_t l;
    char *line, *satline, *obsline;
    satStr_t satid[NSAT]; // Satellite ID strings

    // Maximum number of observation types
    int ntype = 0;
    for (int sys = 0; sys < NSYS; sys++) {
        if (rnxObs->header.nObsType[sys] > ntype) {
            ntype = rnxObs->header.nObsType[sys];
        }
    }

    // Number of lines per satellite
    int nlps = (ntype + 4) / 5;

    // First pass: count epochs and total observations
    for (l = startLine; l < buffer->nline; l++) {

        // Get line from buffer
        if (!(line = GetLine(buffer, l, &len))) return 0;

        // Skip comment lines
        if (LineContains(line, len, "COMMENT", 60)) continue;

        // Check epoch line
        // v2 format: position 29-31 has length > 29, position 3 is space, position 29 <= '1'
        if (len < 30 || line[2] == ' ' || line[3] != ' ' || line[28] > '1') continue;

        // Count epoch
        nepoch++;

        // Get number of satellites in this epoch
        int nsat;
        if (sscanf(line + 29, "%d", &nsat) != 1 || nsat <= 0) continue;
        total += nsat;

        // Number of extended satellite list lines
        int nles = (nsat + 11) / 12 - 1; // Ceiling division

        // Skip to next epoch
        l += (nles + nsat * nlps);
    }

    if (nepoch == 0) return 1; // No observations found

    // Allocate memory for observation body
    rnxObs->body = (rnxObsBody_t *)malloc(total * sizeof(rnxObsBody_t));
    if (!rnxObs->body) return 0;
    rnxObs->n = total;

    // Clear memory
    memset(rnxObs->body, 0, total * sizeof(rnxObsBody_t));

    // Allocate record buffer (reused for all satellites)
    char *record = (char *)malloc((ntype * 16 + 1) * sizeof(char));
    if (!record) return 0;

    // Second pass: parse actual observations
    int oidx = 0;
    for (l = startLine; l < buffer->nline; l++) {

        // Get line from buffer
        if (!(line = GetLine(buffer, l, &len))) return 0;

        // Skip comment lines
        if (LineContains(line, len, "COMMENT", 60)) continue;

        // Check epoch line
        if (len < 30 || line[2] == ' ' || line[3] != ' ' || line[28] > '1') continue;

        // Parse epoch time
        cal_t cal;
        if (sscanf(line, "%2d %2d %2d %2d %2d %10lf",
                  &cal.year, &cal.mon, &cal.day,
                  &cal.hour, &cal.min, &cal.sec) != 6) continue;

        // Convert 2-digit year to 4-digit year
        if (cal.year < 80) {
            cal.year += 2000;
        } else {
            cal.year += 1900;
        }

        // Get number of satellites
        int nsat;
        if (sscanf(line + 29, "%d", &nsat) != 1 || nsat <= 0) continue;

        // Number of extended satellite list lines
        int nles = (nsat + 11) / 12 - 1;

        // Get satellite list
        for (int i = 0; i < nles + 1; i++) {

            // Get line from buffer
            if (!(satline = GetLine(buffer, l + i, &len))) return 0;

            // Get satellite list
            int startCol = 32;
            int nsati = (i == nles) ? (nsat - 12 * i) : 12;

            for (int j = 0; j < nsati; j++) {
                int satidx = 12 * i + j;
                if (satidx >= nsat) break;

                if (len >= startCol + j * 3 + 3) {

                    // Extract satellite ID (3 characters)
                    strncpy(satid[satidx].str, satline + startCol + j * 3, 3);
                    satid[satidx].str[3] = '\0';

                    // Replace spaces with '0' for PRN
                    if (satid[satidx].str[1] == ' ') satid[satidx].str[1] = '0';
                    if (satid[satidx].str[2] == ' ') satid[satidx].str[2] = '0';
                }
            }
        }

        // Parse observations for each satellite
        for (int s = 0; s < nsat && oidx < total; s++) {

            // Set epoch time and satellite ID
            rnxObs->body[oidx].cal = cal;
            strncpy(rnxObs->body[oidx].satStr.str, satid[s].str, 3);
            rnxObs->body[oidx].satStr.str[3] = '\0';

            // Initialize observation arrays
            for (int k = 0; k < MAX_OBSTYPE; k++) {
                rnxObs->body[oidx].obs[k] = 0.0;
                rnxObs->body[oidx].lli[k] = 0;
            }

            // Parse observation lines for this satellite
            // Initialize record with default pattern
            for (int t = 0; t < ntype; t++) {
                strncpy(record + t * 16, "         0.00000", 16);
            }
            record[ntype * 16] = '\0';

            // Fill record with actual observation data
            for (int o = 0; o < nlps; o++) {

                // Get observation line
                if (!(obsline = GetLine(buffer, l + 1 + nles + s * nlps + o, &len))) continue;

                // Copy observation data to record
                int startPos = 80 * o;
                int copyLen = len;
                if (startPos + copyLen <= ntype * 16) {
                    strncpy(record + startPos, obsline, copyLen);
                }
            }

            // Replace empty fields into zeros
            const char defaultField[17] = "         0.00000";
            for (int i = 0; i < ntype * 16; i++) {
                if (record[i] == ' ') {
                    record[i] = defaultField[i % 16];
                }
            }

            // Parse observations from record using RINEX v2 16-character field format
            for (int t = 0; t < ntype && t < MAX_OBSTYPE; t++) {

                int fieldStart = t * 16;

                // Parse observation value (first 14 characters of 16-char field)
                char obsStr[15];
                strncpy(obsStr, record + fieldStart, 14);
                obsStr[14] = '\0';

                // Convert string to float
                rnxObs->body[oidx].obs[t] = strtod(obsStr, NULL);

                // Parse LLI (15th character)
                char lliChar = record[fieldStart + 14];
                if (lliChar >= '0' && lliChar <= '9') {
                    rnxObs->body[oidx].lli[t] = lliChar - '0';
                } else {
                    rnxObs->body[oidx].lli[t] = 0;
                }
            }

            oidx++;
        }

        // Skip observation lines
        l += (nles + nsat * nlps);
    }

    // Free record buffer
    free(record);

    // Update actual number of parsed observations
    rnxObs->n = oidx;

    return 1;
}

// Read RINEX v3 observation file body
static int ReadRnxObsBodyV3(const buffer_t *buffer, rnxObs_t *rnxObs, size_t startLine)
{
    if (!buffer || !rnxObs) return 0;

    int len, nepoch = 0, total = 0;
    size_t l;
    char *line;

    // Maximum number of observation types across all systems
    int ntype = 0;
    for (int sys = 0; sys < NSYS; sys++) {
        if (rnxObs->header.nObsType[sys] > ntype) {
            ntype = rnxObs->header.nObsType[sys];
        }
    }

    // First pass: count epochs and total observations
    for (l = startLine; l < buffer->nline; l++) {

        // Get line from buffer
        if (!(line = GetLine(buffer, l, &len))) return 0;

        // Skip comment lines
        if (LineContains(line, len, "COMMENT", 60)) continue;

        // Check epoch line (v3 format: starts with '>', length > 35)
        if (len <= 35 || line[0] != '>') continue;

        // Count epoch
        nepoch++;

        // Get number of satellites in this epoch
        int nsat;
        if (sscanf(line + 32, "%d", &nsat) != 1 || nsat <= 0) continue;
        total += nsat;

        // Skip satellite observation lines
        l += nsat;
    }

    if (nepoch == 0) return 1; // No observations found

    // Allocate memory for observation body
    rnxObs->body = (rnxObsBody_t *)malloc(total * sizeof(rnxObsBody_t));
    if (!rnxObs->body) return 0;
    rnxObs->n = total;

    // Allocate record buffer (reused for all satellites)
    char *record = (char *)malloc((ntype * 16 + 1) * sizeof(char));
    if (!record) return 0;

    // Second pass: parse actual observations
    int oidx = 0;
    for (l = startLine; l < buffer->nline; l++) {

        // Get line from buffer
        if (!(line = GetLine(buffer, l, &len))) return 0;

        // Skip comment lines
        if (LineContains(line, len, "COMMENT", 60)) continue;

        // Check epoch line (v3 format: starts with '>', length > 35)
        if (len <= 35 || line[0] != '>') continue;

        // Parse epoch time
        cal_t cal;
        if (sscanf(line + 2, "%4d %2d %2d %2d %2d %10lf",
                  &cal.year, &cal.mon, &cal.day,
                  &cal.hour, &cal.min, &cal.sec) != 6) continue;

        // Get number of satellites
        int nsat;
        if (sscanf(line + 32, "%d", &nsat) != 1 || nsat <= 0) continue;

        // Parse observations for each satellite
        for (int s = 0; s < nsat && oidx < total; s++) {

            // Get satellite observation line
            char *obsline;
            if (!(obsline = GetLine(buffer, l + 1 + s, &len))) return 0;

            // Check minimum line length (satellite ID + some data)
            if (len < 3) continue;

            // Set epoch time and satellite ID
            rnxObs->body[oidx].cal = cal;
            strncpy(rnxObs->body[oidx].satStr.str, obsline, 3);
            rnxObs->body[oidx].satStr.str[3] = '\0';

            // Initialize observation arrays
            for (int k = 0; k < MAX_OBSTYPE; k++) {
                rnxObs->body[oidx].obs[k] = 0.0;
                rnxObs->body[oidx].lli[k] = 0;
            }

            // Initialize record with default pattern
            for (int t = 0; t < ntype; t++) {
                strncpy(record + t * 16, "         0.00000", 16);
            }
            record[ntype * 16] = '\0';

            // Copy observation data from line (starting from position 4)
            if (len > 3) {
                int dataLen = len - 3;
                int copyLen = (dataLen < ntype * 16) ? dataLen : ntype * 16;
                strncpy(record, obsline + 3, copyLen);
            }

            // Replace empty fields with zeros
            const char defaultField[17] = "         0.00000";
            for (int i = 0; i < ntype * 16; i++) {
                if (record[i] == ' ') {
                    record[i] = defaultField[i % 16];
                }
            }

            // Validate record format (check for decimal points at expected positions)
            int validRecord = 1;
            for (int t = 0; t < ntype; t++) {
                int decimalPos = t * 16 + 10;
                if (decimalPos < ntype * 16 && record[decimalPos] != '.') {
                    validRecord = 0;
                    break;
                }
            }

            if (!validRecord) {
                oidx++;
                continue;
            }

            // Parse observations from record using RINEX v3 16-character field format
            for (int t = 0; t < ntype && t < MAX_OBSTYPE; t++) {

                int fieldStart = t * 16;

                // Parse observation value (first 14 characters of 16-char field)
                char obsStr[15];
                strncpy(obsStr, record + fieldStart, 14);
                obsStr[14] = '\0';

                // Convert string to float
                rnxObs->body[oidx].obs[t] = strtod(obsStr, NULL);

                // Parse LLI (15th character)
                char lliChar = record[fieldStart + 14];
                if (lliChar >= '0' && lliChar <= '9') {
                    rnxObs->body[oidx].lli[t] = lliChar - '0';
                } else {
                    rnxObs->body[oidx].lli[t] = 0;
                }
            }

            oidx++;
        }

        // Skip satellite observation lines
        l += nsat;
    }

    // Free record buffer
    free(record);

    // Update actual number of parsed observations
    rnxObs->n = oidx;

    return 1;
}

// Read RINEX observation file body
static int ReadRnxObsBody(const buffer_t *buffer, rnxObs_t *rnxObs)
{
    // Check if the parameters are valid
    if (!buffer || !rnxObs) return 0;

    // Initialize variables
    int len;
    size_t l;
    char *line;

    // Skip header lines and find "END OF HEADER"
    for (l = 0; l < buffer->nline; l++) {

        // Get line from buffer
        if (!(line = GetLine(buffer, l, &len))) return 0;

        // Skip header lines and find "END OF HEADER"
        if (LineContains(line, len, "END OF HEADER", 60)) {
            l++;
            break;
        }
    }

    // Parse observations based on RINEX version
    int ver = (int)rnxObs->header.ver;

    if (ver == 2) {
        return ReadRnxObsBodyV2(buffer, rnxObs, l);
    }
    else if (ver == 3) {
        return ReadRnxObsBodyV3(buffer, rnxObs, l);
    }

    return 0; // Unsupported version
}

// Add and arrange observation data to observation structure
static int ArrangeObs(obss_t *obs, rnxObs_t *rnxObs, int rcvidx)
{
    // Check if the parameters are valid
    if (!obs || !rnxObs) return 0;

    // Check if the receiver index is valid
    if (rcvidx <= 0 || rcvidx > NRCV) return 0;

    // Process each observation epoch
    for (int i = 0; i < rnxObs->n; i++) {

        // Convert satellite ID to satellite number
        int sat = Str2Sat(rnxObs->body[i].satStr);
        if (sat <= 0) continue;

        // Get satellite system
        int prn;
        int sys = Sat2Prn(sat, &prn);
        if (sys <= 0 || sys > NSYS) continue;

        // Convert calendar time to standard time
        double time = Cal2Time(rnxObs->body[i].cal);

        // Get number of observation types for this system
        int nObsType = rnxObs->header.nObsType[sys - 1];
        if (nObsType <= 0) continue;

        // Create observation structure
        obs_t newObs;
        memset(&newObs, 0, sizeof(obs_t));

        // Set common fields (time, sat, rcvidx)
        newObs.time = time;
        newObs.rcv = rcvidx;
        newObs.sat = sat;

        // Initialize valid flag
        int validFlag = 0;

        // Process each observation type for this system
        for (int t = 0; t < nObsType && t < MAX_OBSTYPE; t++) {

            // Get observation type string (e.g., "C1C", "L1C", "D1C", "S1C")
            char *obsTypeStr = rnxObs->header.obsType[sys - 1][t];
            if (strlen(obsTypeStr) < 3) continue;

            // Extract frequency and attribute (e.g., "1C" from "C1C")
            char freqAttr[3];
            strncpy(freqAttr, obsTypeStr + 1, 2);
            freqAttr[2] = '\0';

            // Convert to observation code
            codeStr_t codeStr;
            sprintf(codeStr.str, "L%s", freqAttr);  // Start with "L" prefix for code conversion
            int code = Str2Code(codeStr);
            if (code <= 0) continue;

            // Get frequency index for this code
            int fidx = Code2Fidx(sys, code);
            if (fidx <= 0) continue;

            // Get observation value
            double obsValue = rnxObs->body[i].obs[t];
            int lliValue = rnxObs->body[i].lli[t];

            // Skip if observation is zero (invalid)
            if (obsValue == 0.0) continue;

            // Set observation based on type
            char obsType = obsTypeStr[0];
            switch (obsType) {
                case 'C':  // Pseudorange
                    newObs.code[fidx - 1] = code;
                    newObs.P   [fidx - 1] = obsValue;
                    validFlag = 1;  // Set valid flag
                    break;

                case 'L':  // Carrier phase
                    newObs.code[fidx - 1] = code;
                    newObs.L   [fidx - 1] = obsValue;
                    newObs.LLI [fidx - 1] = lliValue;
                    validFlag = 1;  // Set valid flag
                    break;

                case 'D':  // Doppler
                    newObs.code[fidx - 1] = code;
                    newObs.D   [fidx - 1] = obsValue;
                    validFlag = 1;  // Set valid flag
                    break;

                case 'S':  // Signal strength
                    newObs.code[fidx - 1] = code;
                    newObs.SNR [fidx - 1] = obsValue;
                    validFlag = 1;  // Set valid flag
                    break;

                default:
                    continue;  // Unknown observation type - no validFlag set
            }
        }

        // Add observation to the observation structure if valid
        if (validFlag) {
            if (!AddObs(obs, &newObs)) {
                break;  // Failed to add observation, stop processing but keep existing data
            }
        }
    }

    return 1;
}

// Helper function to adjust time considering week handover
static double AdjWeek(double t, double t0)
{
    double dt = t - t0;
    if (dt > 302400.0) t -= 604800.0;
    else if (dt < -302400.0) t += 604800.0;
    return t;
}

// Helper function to adjust time considering day handover
static double AdjDay(double t, double t0)
{
    double dt = t - t0;
    if (dt > 43200.0) t -= 86400.0;
    else if (dt < -43200.0) t += 86400.0;
    return t;
}

// Read RINEX navigation file header (1: success, 0: failure)
static int ReadRnxNavHeader(const buffer_t *buffer, rnxNav_t *rnxNav)
{
    if (!buffer || !rnxNav) return 0;

    char *line;
    int len, endFlag = 0;
    size_t l;
    double ver;

    for (l = 0; l < buffer->nline && !endFlag; l++) {

        // Get line from buffer
        if (!(line = GetLine(buffer, l, &len))) continue;

        // Parse header
        if (LineContains(line, len, "RINEX VERSION / TYPE", 60)) {

            // Set version and system
            if (sscanf(line, "%lf", &ver) != 1) return 0;
            rnxNav->header.ver = ver;

            if (ver < 3.0) {
                if (len > 20) {
                    switch (line[20]) {
                        case 'N': rnxNav->header.sys = STR_GPS; break; // GPS
                        case 'G': rnxNav->header.sys = STR_GLO; break; // GLONASS
                        default: return 0;
                    }
                }
            } else {
                if (len > 40) rnxNav->header.sys = line[40];
            }
        }
        else if (LineContains(line, len, "ION ALPHA", 60)) { // For ver 2 GPS
            int sysidx = Str2Sys(STR_GPS);
            if (sysidx > 0) {
                // Create a copy of the line and replace D with E
                char line_copy[512];
                strncpy(line_copy, line, len < 511 ? len : 511);
                line_copy[len < 511 ? len : 511] = '\0';

                // Replace D or d with E in the line copy
                for (int i = 0; line_copy[i] != '\0'; i++) {
                    if (line_copy[i] == 'D' || line_copy[i] == 'd') {
                        line_copy[i] = 'E';
                    }
                }

                sscanf(line_copy, "%lf %lf %lf %lf",
                       &rnxNav->header.iono[sysidx - 1][0],
                       &rnxNav->header.iono[sysidx - 1][1],
                       &rnxNav->header.iono[sysidx - 1][2],
                       &rnxNav->header.iono[sysidx - 1][3]);
            }
        }
        else if (LineContains(line, len, "ION BETA", 60)) { // For ver 2 GPS
            int sysidx = Str2Sys(STR_GPS);
            if (sysidx > 0) {
                // Create a copy of the line and replace D with E
                char line_copy[512];
                strncpy(line_copy, line, len < 511 ? len : 511);
                line_copy[len < 511 ? len : 511] = '\0';

                // Replace D or d with E in the line copy
                for (int i = 0; line_copy[i] != '\0'; i++) {
                    if (line_copy[i] == 'D' || line_copy[i] == 'd') {
                        line_copy[i] = 'E';
                    }
                }

                sscanf(line_copy, "%lf %lf %lf %lf",
                       &rnxNav->header.iono[sysidx - 1][4],
                       &rnxNav->header.iono[sysidx - 1][5],
                       &rnxNav->header.iono[sysidx - 1][6],
                       &rnxNav->header.iono[sysidx - 1][7]);
            }
        }
        else if (LineContains(line, len, "IONOSPHERIC CORR", 60)) { // For ver 3
            char type_str[5];
            double params[4];

            // Extract type and parameters
            strncpy(type_str, line, 4);
            type_str[4] = '\0';

            // Create a copy of the line and replace D with E
            char line_copy[512];
            strncpy(line_copy, line, len < 511 ? len : 511);
            line_copy[len < 511 ? len : 511] = '\0';

            // Replace D or d with E in the line copy
            for (int i = 0; line_copy[i] != '\0'; i++) {
                if (line_copy[i] == 'D' || line_copy[i] == 'd') {
                    line_copy[i] = 'E';
                }
            }

            sscanf(line_copy + 5, "%lf %lf %lf %lf", &params[0], &params[1], &params[2], &params[3]);

            int sysidx = 0;
            if (strncmp(type_str, "GPSA", 4) == 0 || strncmp(type_str, "GPSB", 4) == 0) {
                sysidx = Str2Sys(STR_GPS);
            } else if (strncmp(type_str, "BDSA", 4) == 0 || strncmp(type_str, "BDSB", 4) == 0) {
                sysidx = Str2Sys(STR_BDS);
            } else if (strncmp(type_str, "QZSA", 4) == 0 || strncmp(type_str, "QZSB", 4) == 0) {
                sysidx = Str2Sys(STR_QZS);
            } else if (strncmp(type_str, "IRNA", 4) == 0 || strncmp(type_str, "IRNB", 4) == 0) {
                sysidx = Str2Sys(STR_IRN);
            } else if (strncmp(type_str, "GAL", 3) == 0) {
                sysidx = Str2Sys(STR_GAL);
            }

            if (sysidx > 0) {
                if (strncmp(type_str, "GAL", 3) == 0) { // NeQuick-G has 3 parameters
                    rnxNav->header.iono[sysidx - 1][0] = params[0];
                    rnxNav->header.iono[sysidx - 1][1] = params[1];
                    rnxNav->header.iono[sysidx - 1][2] = params[2];
                } else if (type_str[3] == 'A') { // Klobuchar-like alpha
                    rnxNav->header.iono[sysidx - 1][0] = params[0];
                    rnxNav->header.iono[sysidx - 1][1] = params[1];
                    rnxNav->header.iono[sysidx - 1][2] = params[2];
                    rnxNav->header.iono[sysidx - 1][3] = params[3];
                } else if (type_str[3] == 'B') { // Klobuchar-like beta
                    rnxNav->header.iono[sysidx - 1][4] = params[0];
                    rnxNav->header.iono[sysidx - 1][5] = params[1];
                    rnxNav->header.iono[sysidx - 1][6] = params[2];
                    rnxNav->header.iono[sysidx - 1][7] = params[3];
                }
            }
        }
        else if (LineContains(line, len, "END OF HEADER", 60)) {
            endFlag = 1;
        }
    }

    return endFlag;
}

// Read RINEX navigation file body (1: success, 0: failure)
static int ReadRnxNavBody(const buffer_t *buffer, rnxNav_t *rnxNav)
{
    // Check if the parameters are valid
    if (!buffer || !rnxNav) return 0;

    int ver = (int)rnxNav->header.ver;
    int v3 = (ver == 3);
    char *line;
    int len;
    size_t l;

    // Find start of body
    size_t startLine = 0;
    for (l = 0; l < buffer->nline; l++) {

        // Get line from buffer
        if (!(line = GetLine(buffer, l, &len))) return 0;

        // Check if the line contains "END OF HEADER"
        if (LineContains(line, len, "END OF HEADER", 60)) {
            startLine = l + 1;
            break;
        }
    }

    // Header end not found
    if (startLine == 0) return 0;

    // First pass: count navigation messages
    int total = 0;
    for (l = startLine; l < buffer->nline; l++) {
        if (!(line = GetLine(buffer, l, &len))) continue;

        cal_t cal; // dummy
        int sys, prn;

        // Common epoch line format check
        if (len < 4 || line[1 + v3] == ' ' || line[2 + v3] != ' ') continue;

        if (v3) {
            // RINEX v3: Check if first character is valid system identifier
            sys = Str2Sys(line[0]);
            if (sys == 0) continue;

        } else { // v2

            // RINEX v2: Try to parse PRN from beginning
            if (sscanf(line, "%d", &prn) == 1) {

                // Standard case: line starts with PRN number
                sys = Str2Sys(rnxNav->header.sys);

            } else if (Str2Sys(rnxNav->header.sys) == Str2Sys(STR_GAL) &&
                       Str2Sys(line[0]) == Str2Sys(STR_GAL) && 0) { // TBD

                // Currently v2 Galileo ephemeris is not supported
                // Special case: Galileo with "EXX" format in v2 (TBD)
                sys = Str2Sys(STR_GAL);

            } else {
                // Invalid format
                continue;
            }
        }

        // Determine number of orbit data lines based on system
        int nlo;
        switch (Sys2Str(sys)) {
            case STR_GLO:
            case STR_SBS:
                nlo = 3;
                break;
            default:
                nlo = 7;
                break;
        }

        total++;
        l += nlo;
    }

    if (total == 0) return 1;

    // Allocate memory
    rnxNav->body = (rnxNavBody_t *)malloc(total * sizeof(rnxNavBody_t));
    if (!rnxNav->body) return 0;
    rnxNav->n = total;

    // Clear memory
    memset(rnxNav->body, 0, total * sizeof(rnxNavBody_t));

    // Second pass: parse navigation messages
    int nidx = 0;
    for (l = startLine; l < buffer->nline && nidx < total; l++) {

        // Get line from buffer
        if (!(line = GetLine(buffer, l, &len))) continue;

        // Get body pointer
        rnxNavBody_t *body = &rnxNav->body[nidx];

        // Common epoch line format check
        if (len < 4 || line[1 + v3] == ' ' || line[2 + v3] != ' ') continue;

        // Step 1: Parse epoch line (satStr and calendar only)
        int valid_epoch = 0;
        if (v3) {
            // RINEX v3: Parse system identifier and epoch data
            int sys = Str2Sys(line[0]);
            if (sys == 0) continue;

            if (sscanf(line, "%3s %4d %2d %2d %2d %2d %lf",
                       body->satStr.str, &body->cal.year, &body->cal.mon, &body->cal.day,
                       &body->cal.hour, &body->cal.min, &body->cal.sec) >= 7) {
                valid_epoch = 1;
            }

        } else { // v2
            // RINEX v2: Try to parse PRN from beginning
            int prn;
            if (sscanf(line, "%d %2d %2d %2d %2d %2d %lf",
                       &prn, &body->cal.year, &body->cal.mon, &body->cal.day,
                       &body->cal.hour, &body->cal.min, &body->cal.sec) >= 7) {

                // Standard case: line starts with PRN number
                if (body->cal.year < 80) body->cal.year += 2000; else body->cal.year += 1900;
                sprintf(body->satStr.str, "%c%02d", rnxNav->header.sys, prn);
                valid_epoch = 1;

            } else if (Str2Sys(rnxNav->header.sys) == Str2Sys(STR_GAL) &&
                       Str2Sys(line[0]) == Str2Sys(STR_GAL) && 0) { // TBD

                // Currently v2 Galileo ephemeris is not supported
                // Special case: Galileo with "EXX" format in v2 (TBD)
                if (sscanf(line, "%3s %2d %2d %2d %2d %2d %lf",
                           body->satStr.str, &body->cal.year, &body->cal.mon, &body->cal.day,
                           &body->cal.hour, &body->cal.min, &body->cal.sec) >= 7) {

                    if (body->cal.year < 80) body->cal.year += 2000; else body->cal.year += 1900;
                    valid_epoch = 1;
                }
            }
        }

        // Check if epoch parsing was successful
        if (!valid_epoch) continue;

        // Step 2: Parse clock parameters (handle D->E conversion)
        char field[20];
        for (int i = 0; i < 3; i++) {
            int offset = (v3 ? 23 : 22) + i * 19;  // Clock field positions
            if (offset + 19 > len) {
                body->clock[i] = 0.0;
                continue;
            }

            // Extract clock field
            strncpy(field, line + offset, 19);
            field[19] = '\0';

            // Replace D or d with E for strtod
            for (int k = 0; field[k] != '\0'; k++) {
                if (field[k] == 'D' || field[k] == 'd') {
                    field[k] = 'E';
                }
            }

            body->clock[i] = strtod(field, NULL);
        }

        // Number of lines of orbit data
        int sys = Str2Sys(body->satStr.str[0]);
        int nlo;
        switch (Sys2Str(sys)) {
            case STR_GLO:
            case STR_SBS:
                nlo = 3;
                break;
            default:
                nlo = 7;
                break;
        }

        // Parse orbit data lines
        for (int i = 0; i < nlo; i++) {

            // Get orbit data line
            char *orbitline;
            if (!(orbitline = GetLine(buffer, l + 1 + i, &len))) break;

            char field[20];
            for (int j = 0; j < 4; j++) {
                int orbit_idx = i * 4 + j;
                if (orbit_idx >= 28) break;

                int offset = (v3 ? 4 : 3) + j * 19;
                if (offset + 19 > len) {
                    body->orbit[orbit_idx] = 0.0;
                    continue;
                }

                // Parse orbit value
                strncpy(field, orbitline + offset, 19);
                field[19] = '\0';

                // Replace D or d with E for strtod
                for (int k = 0; field[k] != '\0'; k++) {
                    if (field[k] == 'D' || field[k] == 'd') {
                        field[k] = 'E';
                    }
                }

                body->orbit[orbit_idx] = strtod(field, NULL);
            }
        }

        // Update index
        nidx++;

        // Skip orbit data lines
        l += nlo;
    }

    // Update with actual count
    rnxNav->n = nidx;

    return 1;
}

// Add and arrange navigation data to navigation structure
static int ArrangeNav(nav_t *nav, rnxNav_t *rnxNav)
{
    // Check if the parameters are valid
    if (!nav || !rnxNav) return 0;

    // Initialize ephemeris structure
    eph_t eph;

    // Process each navigation message
    for (int i = 0; i < rnxNav->n; i++) {

        // Get body pointer
        rnxNavBody_t *body = &rnxNav->body[i];

        // Get satellite index
        int sat = Str2Sat(body->satStr);
        if (sat <= 0) continue;

        // Get system and PRN
        int prn;
        int sys = Sat2Prn(sat, &prn);
        if (sys <= 0) continue;

        // Clear memory
        memset(&eph, 0, sizeof(eph_t));
        eph.sat = sat;

        // TOC in calendar format from body
        double toc = Cal2Time(body->cal);
        int week;
        double tocs = Time2Gpst(toc, &week);

        // Process each system
        char sysStr = Sys2Str(sys);
        switch (sysStr) {
            case STR_GPS:
            case STR_GAL:
            case STR_QZS:
            case STR_BDS:
            case STR_IRN:

                // Clock parameters
                eph.af0 = body->clock[0];
                eph.af1 = body->clock[1];
                eph.af2 = body->clock[2];

                // Orbit parameters
                eph.IODE = (int)body->orbit[0];
                eph.crs  = body->orbit[1];
                eph.deln = body->orbit[2];
                eph.M0   = body->orbit[3];
                eph.cuc  = body->orbit[4];
                eph.e    = body->orbit[5];
                eph.cus  = body->orbit[6];
                eph.A    = body->orbit[7] * body->orbit[7];
                eph.toes = body->orbit[8];
                eph.cic  = body->orbit[9];
                eph.OMG0 = body->orbit[10];
                eph.cis  = body->orbit[11];
                eph.i0   = body->orbit[12];
                eph.crc  = body->orbit[13];
                eph.omg  = body->orbit[14];
                eph.OMGd = body->orbit[15];
                eph.iodt = body->orbit[16];

                // Code on L2 (G,J) and data source (E)
                if (sysStr == STR_GPS || sysStr == STR_QZS) {
                    eph.code = (int)body->orbit[17];
                }
                else if (sysStr == STR_GAL) {
                    eph.data = (int)body->orbit[17];
                }

                // GPS week (no week rollover) (G,E,C,J,I) (C: BDS week)
                eph.week = (int)body->orbit[18];

                // L2P data flag (G,J)
                if (sysStr == STR_GPS || sysStr == STR_QZS) {
                    eph.flag = (int)body->orbit[19];
                }

                // Satellite accuracy (G,E,C,J,I)
                if (sysStr == STR_GAL) {
                    eph.sva = Sisa2Idx(body->orbit[20]);
                }
                else {
                    eph.sva = Ura2Idx(body->orbit[20]);
                }

                // Satellite health (G,E,C,J,I)
                eph.svh = (int)body->orbit[21];

                // TGD (G,J,I), BGD E5a/E1 (E), TGD1 B1/B3 (C)
                eph.tgd[0] = body->orbit[22];

                // IODC (G,J,I), BGD E5b/E1 (E), TGD2 B2/B3 (C)
                if (sysStr == STR_GAL || sysStr == STR_BDS) {
                    eph.tgd[1] = body->orbit[23];
                }
                else {
                    eph.IODC = (int)body->orbit[23];
                }

                // Time of transmission
                eph.ttrs = body->orbit[24];

                // Fit interval (G,J), AODC (C)
                if (sysStr == STR_GPS) {
                    eph.fit = body->orbit[25] == 0.0 ? 4.0 : body->orbit[25];
                }
                else if (sysStr == STR_QZS) {
                    eph.fit = body->orbit[25] == 0.0 ? 2.0 : body->orbit[25] == 1.0 ? 4.0 : body->orbit[25];
                }
                else if (sysStr == STR_BDS) {
                    eph.AODC = (int)body->orbit[25];
                }

                // Convert week and toc, toe, ttr to standard time
                if (sysStr == STR_BDS) {
                    eph.toc = Bdt2Gpst(Gpst2Time(week, tocs));          // GPS week and toc of BDS time -> GPS time
                    eph.toe = Bdt2Gpst(Bdt2Time(eph.week, eph.toes));   // BDS week and toe of BDS time -> GPS time
                    eph.ttr = Bdt2Gpst(Bdt2Time(eph.week, eph.ttrs));   // BDS week and ttr of BDS time -> GPS time
                }
                else {
                    eph.toc = Gpst2Time(week, tocs);                    // GPS week and toc of GPS time -> GPS time
                    eph.toe = Gpst2Time(eph.week, eph.toes);            // GPS week and toe of GPS time -> GPS time
                    eph.ttr = Gpst2Time(eph.week, eph.ttrs);            // GPS week and ttr of GPS time -> GPS time
                }

                // Adjust week handover
                eph.toe = AdjWeek(eph.toe, eph.toc);
                eph.ttr = AdjWeek(eph.ttr, eph.toc);

                // Add ephemeris to navigation structure
                if (!AddEph(nav->ephs + sat - 1, &eph)) break;
                break;

            case STR_GLO:
                // GLONASS ephemeris

                // Clock parameters
                eph.taun = -body->clock[0];
                eph.gamn =  body->clock[1];

                // Orbit params are in km, convert to m
                eph.pos[0] = body->orbit[0] * 1000.0;
                eph.vel[0] = body->orbit[1] * 1000.0;
                eph.acc[0] = body->orbit[2] * 1000.0;
                eph.svh    = (int)body->orbit[3];
                eph.pos[1] = body->orbit[4] * 1000.0;
                eph.vel[1] = body->orbit[5] * 1000.0;
                eph.acc[1] = body->orbit[6] * 1000.0;
                eph.frq    = (int)body->orbit[7];
                eph.pos[2] = body->orbit[8] * 1000.0;
                eph.vel[2] = body->orbit[9] * 1000.0;
                eph.acc[2] = body->orbit[10] * 1000.0;
                eph.age    = (int)body->orbit[11];

                // Compute toc rounded by 15 minutes in UTC time
                toc = Gpst2Time(week, round((tocs + 450.0) / 900.0) * 900.0);
                int dow = (int)floor(tocs / 86400.0);

                // Time of day
                double tod = body->clock[2]; // Time of day
                if ((int)(rnxNav->header.ver) == 3) {
                    tod = fmod(tod, 86400.0);
                }

                // Convert UTC to GPST
                eph.toc = toc;
                eph.toe = toc; // For GLO, toe is the same as toc
                eph.ttr = Utc2Gpst(AdjDay(Gpst2Time(week, dow * 86400.0 + tod), toc));

                // Compute IODE = tb(7 bits), tb = index of UTC + 3h within current day
                eph.IODE = (int)round(fmod(tocs + 10800.0, 86400.0) / 900.0);

                // Add ephemeris
                if (!AddEph(nav->ephs + sat - 1, &eph)) break;
                break;

            case STR_SBS:
                // SBAS ephemeris

                // Clock parameters
                eph.af0  = body->clock[0];
                eph.af1  = body->clock[1];
                eph.ttrs = body->clock[2];

                // Orbit params are in km, convert to m
                eph.pos[0] = body->orbit[0] * 1000.0;
                eph.vel[0] = body->orbit[1] * 1000.0;
                eph.acc[0] = body->orbit[2] * 1000.0;
                eph.svh    = (int)body->orbit[3];
                eph.pos[1] = body->orbit[4] * 1000.0;
                eph.vel[1] = body->orbit[5] * 1000.0;
                eph.acc[1] = body->orbit[6] * 1000.0;
                eph.sva    = Ura2Idx(body->orbit[7]); // URA index
                eph.pos[2] = body->orbit[8] * 1000.0;
                eph.vel[2] = body->orbit[9] * 1000.0;
                eph.acc[2] = body->orbit[10] * 1000.0;
                eph.IODE   = (int)body->orbit[11]; // IODN

                // Time conversion
                eph.toc = Gpst2Time(week, tocs);                    // GPS week and toc of GPS time -> GPS time
                eph.toe = eph.toc;                                  // SBAS toe is the same as toc
                eph.ttr = Gpst2Time(week, body->clock[2]);          // GPS week and ttr of GPS time -> GPS time

                // Adjust week handover
                eph.ttr = AdjWeek(eph.ttr, eph.toe);

                // Add ephemeris
                if (!AddEph(nav->ephs + sat - 1, &eph)) break;
                break;
        }
    }

    return 1;
}

// =============================================================================
// RINEX check functions
// =============================================================================

// Check if file name is the RINEX obs file name
int IsRinexObs(const char *filename)
{
    if (!filename) return 0;

    int len = strlen(filename);
    if (len < 4) return 0;

    // Check for .XXo or .XXO pattern (where XX is 2 digits, e.g., .02o, .22o, .25O)
    if ((filename[len-1] == 'o' || filename[len-1] == 'O') && filename[len-4] == '.') {
        char c1 = filename[len-3];
        char c2 = filename[len-2];
        if (c1 >= '0' && c1 <= '9' && c2 >= '0' && c2 <= '9') {
            return 1;
        }
    }

    // Check for .rnx pattern
    if (len >= 4 &&
        filename[len-1] == 'x' &&
        filename[len-2] == 'n' &&
        filename[len-3] == 'r' &&
        filename[len-4] == '.') {
        return 1;
    }

    return 0;
}

// Check if file name is the RINEX navigation file name
int IsRinexNav(const char *filename)
{
    if (!filename) return 0;

    int len = strlen(filename);
    if (len < 4) return 0;

    // Check for pattern (where XX is 2 digits, e.g., .02n, .22n, .25N)
    // .XXn, .XXN
    // .XXg, .XXG
    // .XXl, .XXL
    if ((filename[len-1] == 'n' || filename[len-1] == 'N' ||
         filename[len-1] == 'g' || filename[len-1] == 'G' ||
         filename[len-1] == 'l' || filename[len-1] == 'L') &&
         filename[len-4] == '.') {
        char c1 = filename[len-3];
        char c2 = filename[len-2];
        if (c1 >= '0' && c1 <= '9' && c2 >= '0' && c2 <= '9') {
            return 1;
        }
    }

    // Check for .rnx pattern
    if (len >= 4 &&
        filename[len-1] == 'x' &&
        filename[len-2] == 'n' &&
        filename[len-3] == 'r' &&
        filename[len-4] == '.') {
        return 1;
    }

    return 0;
}

// =============================================================================
// RINEX file read functions
// =============================================================================

// Read RINEX observation file (support version 2.xx and 3.xx)
int ReadRnxObs(nav_t *nav, obss_t *obs, int rcvidx, const char *filename)
{
    // Check if the navigation, observation, and file name are valid
    if (!nav || !obs || !filename) return 0;

    // Check if the receiver index is valid
    if (rcvidx <= 0) return 0;

    // Check file name is the RINEX obs file name
    if (!IsRinexObs(filename)) return 0;

    // Initialize file buffer
    buffer_t buffer;
    if (!InitBuff(&buffer)) return 0;

    // Get file buffer
    if (!GetBuff(filename, &buffer)) {
        FreeBuff(&buffer);
        return 0;
    }

    // Initialize RINEX observation structure
    rnxObs_t rnxObs;
    if (!InitRnxObs(&rnxObs)) {
        FreeBuff(&buffer);
        return 0;
    }

    // Read RINEX observation header
    if (!ReadRnxObsHeader(&buffer, &rnxObs)) {
        FreeRnxObs(&rnxObs);
        FreeBuff(&buffer);
        return 0;
    }

    // Check RINEX version
    int ver = (int)rnxObs.header.ver;
    if (ver < 2 || ver > 3) {
        FreeRnxObs(&rnxObs);
        FreeBuff(&buffer);
        return 0;
    }

    // Read RINEX observation body
    if (!ReadRnxObsBody(&buffer, &rnxObs)) {
        FreeRnxObs(&rnxObs);
        FreeBuff(&buffer);
        return 0;
    }

    // Copy station information to navigation data
    if (rcvidx <= NRCV) {
        nav->sta[rcvidx - 1] = rnxObs.header.sta;
    }

    // Add and arrange observation data to observation structure
    if (!ArrangeObs(obs, &rnxObs, rcvidx)) {
        FreeRnxObs(&rnxObs);
        FreeBuff(&buffer);
        return 0;
    }

    // Free memory
    FreeRnxObs(&rnxObs);
    FreeBuff(&buffer);

    return 1; // Success
}

// Read RINEX navigation file (support version 2.xx and 3.xx)
int ReadRnxNav(nav_t *nav, const char *filename)
{
    // Check if the navigation and file name are valid
    if (!nav || !filename) return 0;

    // Check if file name is the RINEX navigation file name
    if (!IsRinexNav(filename)) return 0;

    // Initialize file buffer
    buffer_t buffer;
    if (!InitBuff(&buffer)) return 0;

    // Get file buffer
    if (!GetBuff(filename, &buffer)) {
        FreeBuff(&buffer);
        return 0;
    }

    // Initialize RINEX navigation structure
    rnxNav_t rnxNav;
    if (!InitRnxNav(&rnxNav)) {
        FreeBuff(&buffer);
        return 0;
    }

    // Read RINEX navigation header
    if (!ReadRnxNavHeader(&buffer, &rnxNav)) {
        FreeRnxNav(&rnxNav);
        FreeBuff(&buffer);
        return 0;
    }

    // Check RINEX version
    int ver = (int)rnxNav.header.ver;
    if (ver < 2 || ver > 3) {
        FreeRnxNav(&rnxNav);
        FreeBuff(&buffer);
        return 0;
    }

    // Check RINEX system
    char sys = rnxNav.header.sys;
    if (sys != 'M' && Str2Sys(sys) == 0) {
        FreeRnxNav(&rnxNav);
        FreeBuff(&buffer);
        return 0;
    }

    // Read RINEX navigation body
    if (!ReadRnxNavBody(&buffer, &rnxNav)) {
        FreeRnxNav(&rnxNav);
        FreeBuff(&buffer);
        return 0;
    }

    // Update broadcast ionospheric parameters
    for (int c = 0; c < NSYS; c++) {

        int anyNonZero = 0;

        for (int j = 0; j < 8; j++) {
            if (rnxNav.header.iono[c][j] != 0.0) {
                anyNonZero = 1;
                break;
            }
        }

        // Only update if any non-zero parameters exist
        if (anyNonZero) {
            for (int j = 0; j < 8; j++) {
                nav->iono[c][j] = rnxNav.header.iono[c][j];
            }
        }
    }

    // Add and arrange navigation data to navigation structure
    if (!ArrangeNav(nav, &rnxNav)) {
        FreeRnxNav(&rnxNav);
        FreeBuff(&buffer);
        return 0;
    }

    // Free memory
    FreeRnxNav(&rnxNav);
    FreeBuff(&buffer);

    return 1; // Success
}

// =============================================================================
// End of file
// =============================================================================
