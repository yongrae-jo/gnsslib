// =============================================================================
// GNSS Library header
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#ifndef GNSSLIB_H
#define GNSSLIB_H

#ifdef __cplusplus
extern "C" {
#endif

// Standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stddef.h>

// GNSS library headers (모든 헤더 포함)
#include "const.h"
#include "common.h"
#include "matrix.h"
#include "obs.h"
#include "ephemeris.h"
#include "files.h"
#include "rinex.h"
#include "option.h"
#include "solution.h"
#include "sig.h"

// =============================================================================
// End of header
// =============================================================================
#ifdef __cplusplus
}
#endif

#endif // GNSSLIB_H
