// =============================================================================
// Option source file
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

#include "option.h"
#include "common.h"

// =============================================================================
// Option functions
// =============================================================================

// Set default option settings
void SetDefaultOpt(opt_t *opt) {
    opt->mode           = PROCMODE_SPP;
    opt->engine         = ENGINE_LSQ;
    opt->nrcv           = 1;
    opt->nfreq          = 1;

    opt->ephopt         = EPHOPT_BRDC;
    opt->posopt         = POSOPT_EST;
    opt->ionoopt        = IONOOPT_BRDC;
    opt->tropoopt       = TROPOOPT_SAAS;
    opt->par            = 0;
    opt->cascade        = 0;
    opt->gloaropt       = 0;
    opt->dynamics       = DYNOPT_OFF;

    opt->maxout         = 5;
    opt->minlock        = 0;

    opt->ts             = 0.0;
    opt->te             = 0.0;

    opt->err            = 3E-3;
    opt->errratio       = 100.0;

    opt->procnoiseAmb   = 1E-8;
    opt->procnoiseTropo = 1E-4;
    opt->procnoiseIono  = 0.0;
    opt->procnoiseHacc  = 1E-2;
    opt->procnoiseVacc  = 1E-3;
    opt->procnoiseDtr   = 0.0;
    opt->procnoiseDts   = 0.0;
    opt->procnoiseIsb   = 0.0;
    opt->procnoiseCbr   = 1E-8;
    opt->procnoiseCbs   = 1E-8;
    opt->procnoisePbr   = 1E-8;
    opt->procnoisePbs   = 1E-8;

    opt->elmask         = 10.0 * D2R;
    opt->maxgdop        = 30.0;

    for (int i = 0; i < NSAT; i++) opt->exsats[i] = 0;

    // Set default excluded satellites for BeiDou GEO (C01-C05, C59-C62)
    int sat;
    if (Str2Sys('C') > 0) {
        if ((sat = Str2Sat((satStr_t){.str = "C01"})) > 0) opt->exsats[sat] = 1; // BDS 2 GEO
        if ((sat = Str2Sat((satStr_t){.str = "C02"})) > 0) opt->exsats[sat] = 1; // BDS 2 GEO
        if ((sat = Str2Sat((satStr_t){.str = "C03"})) > 0) opt->exsats[sat] = 1; // BDS 2 GEO
        if ((sat = Str2Sat((satStr_t){.str = "C04"})) > 0) opt->exsats[sat] = 1; // BDS 2 GEO
        if ((sat = Str2Sat((satStr_t){.str = "C05"})) > 0) opt->exsats[sat] = 1; // BDS 2 GEO
        if ((sat = Str2Sat((satStr_t){.str = "C59"})) > 0) opt->exsats[sat] = 1; // BDS 3 GEO
        if ((sat = Str2Sat((satStr_t){.str = "C60"})) > 0) opt->exsats[sat] = 1; // BDS 3 GEO
        if ((sat = Str2Sat((satStr_t){.str = "C61"})) > 0) opt->exsats[sat] = 1; // BDS 3 GEO
        if ((sat = Str2Sat((satStr_t){.str = "C62"})) > 0) opt->exsats[sat] = 1; // BDS 3 GEO
    }
}
