// =============================================================================
// Satellite ephemeris functions
// =============================================================================

#include "ephemeris.h"
#include "common.h"

// =============================================================================
// Static variables
// =============================================================================

// Broadcast ephemeris type
static int EPHTYPE[NSYS] = {0};


// =============================================================================
// Broadcast ephemeris functions
// =============================================================================

// Get broadcast ephemeris type
int GetEphType(int sys)
{
    if (sys < 1 || sys > NSYS) return -1;

    // Return broadcast ephemeris type
    return EPHTYPE[sys-1];
}

// Set broadcast ephemeris type
void SetEphType(int sys, int type)
{
    if (sys < 1 || sys > NSYS) return;

    // Check if the broadcast ephemeris type is valid
    if (Sys2Str(sys) == STR_GPS) {
        if (type < 0 || type > 1) return;
    }
    else if (Sys2Str(sys) == STR_GLO) {
        if (type < 0 || type > 1) return;
    }
    else if (Sys2Str(sys) == STR_GAL) {
        if (type < 0 || type > 2) return;
    }
    else if (Sys2Str(sys) == STR_BDS) {
        if (type < 0 || type > 1) return;
    }
    else if (Sys2Str(sys) == STR_QZS) {
        if (type < 0 || type > 1) return;
    }
    else if (Sys2Str(sys) == STR_IRN) {
        if (type < 0 || type > 1) return;
    }
    else if (Sys2Str(sys) == STR_SBS) {
        if (type < 0 || type > 1) return;
    }

    // Set broadcast ephemeris type
    EPHTYPE[sys-1] = type;
}

// =============================================================================
// End of file
// =============================================================================
