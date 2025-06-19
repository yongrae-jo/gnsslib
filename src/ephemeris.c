// =============================================================================
// Satellite ephemeris functions
//
// -----------------------------------------------------------------------------
// Yongrae Jo, 0727ggame@sju.ac.kr
// =============================================================================

// Standard library
#include <stddef.h>                     // for NULL
#include <math.h>                       // for sqrt, sin, cos, atan2, fabs
#include <stdlib.h>                     // for realloc, free

// GNSS library
#include "ephemeris.h"
#include "common.h"                     // for common functions
#include "matrix.h"                     // for matrix functions

// =============================================================================
// Macros
// =============================================================================

#define J2_GLO          1.0826257E-3            // 2nd zonal harmonic of geopotential (GLO)

#define SIN_5           -0.08715574274765817    // sin(-5°)
#define COS_5           0.9961946980917455      // cos(-5°)

#define TSTEP           60.0                    // Time step for GLONASS breoadcast ephemeris (s)

#define TOL_KEPLER      1E-13                   // Tolerance for Kepler's equation

// =============================================================================
// Static variables
// =============================================================================

// Broadcast ephemeris type
static int EPHTYPE[NSYS] = {0};

// URA error value
static const double URA_ERR[] = {
      2.40,    3.40,    4.85,    6.85,    9.65,
     13.65,   24.00,   48.00,   96.00,  192.00,
    384.00,  768.00, 1536.00, 3072.00, 6144.00,
};

// Number of URA error values
static const int NURA = sizeof(URA_ERR) / sizeof(URA_ERR[0]);

// =============================================================================
// Static functions
// =============================================================================

// Compute satellite position and clock bias using kepler parameters (1:ok, 0:error)
static int Eph2Pos(double time, const eph_t *eph, double *pos, double *clk, double *var)
{
    // Initialize output
    if (pos) {pos[0] = pos[1] = pos[2] = 0.0;}
    if (clk) {*clk = 0.0;}
    if (var) {*var = 0.0;}

    // Check if the broadcast ephemeris data is valid
    if (!eph) return 0;

    // Check ephemeris validity
    if (eph->A <= 0.0) return 0;

    // Get satellite system and PRN
    int prn;
    int sys = Sat2Prn(eph->sat, &prn);
    if (sys <= 0 || sys > NSYS) return 0;

    // Gravitational constant and earth's angular rate
    double mu, omge;
    switch (Sys2Str(sys)) {
        case STR_GPS: mu = MU_GPS; omge = OMGE_GPS; break;
        case STR_GAL: mu = MU_GAL; omge = OMGE_GAL; break;
        case STR_BDS: mu = MU_BDS; omge = OMGE_BDS; break;
        case STR_QZS: mu = MU_QZS; omge = OMGE_QZS; break;
        case STR_IRN: mu = MU_IRN; omge = OMGE_IRN; break;
        default: return 0;
    }

    // Time from ephemeris reference epoch
    double tk = time - eph->toe;

    // Mean anomaly
    double Mk = eph->M0 + (sqrt(mu / (eph->A * eph->A * eph->A)) + eph->deln) * tk;

    // Kepler's equation (by newton-raphson method)
    double Ek0 = 0.0, Ek = Mk;
    int iter;
    for (iter = 0; fabs(Ek - Ek0) > TOL_KEPLER && iter < MAX_ITER_KEPLER; iter++) {
        Ek0 = Ek;
        Ek -= (Ek - eph->e * sin(Ek) - Mk) / (1.0 - eph->e * cos(Ek));
    }

    // Check convergence
    if (iter >= MAX_ITER_KEPLER) return 0;

    // Trigonometric functions
    double sinEk = sin(Ek);
    double cosEk = cos(Ek);

    // Argument of latitude, radius and inclination
    double uk = atan2(sqrt(1.0 - eph->e * eph->e) * sinEk, cosEk - eph->e) + eph->omg;
    double rk = eph->A * (1.0 - eph->e * cosEk);
    double ik = eph->i0 + eph->iodt * tk;

    // Correct argument of latitude, radius and inclination
    double sin2uk = sin(2.0 * uk);
    double cos2uk = cos(2.0 * uk);

    uk += eph->cus * sin2uk + eph->cuc * cos2uk;
    rk += eph->crs * sin2uk + eph->crc * cos2uk;
    ik += eph->cis * sin2uk + eph->cic * cos2uk;

    // Satellite positions in orbital plane
    double xkp = rk * cos(uk);
    double ykp = rk * sin(uk);

    // Trigonometric functions for inclination
    double sinik = sin(ik);
    double cosik = cos(ik);

    // Check if BeiDou GEO satellite
    if (Sys2Str(sys) == STR_BDS && (prn <= 5 || prn >= 59)) {
        // BeiDou GEO satellite coordinate transformation

        // Correct longitude of ascending node
        double omgk = eph->OMG0 + eph->OMGd * tk - omge * eph->toes;
        double sinomgk = sin(omgk);
        double cosomgk = cos(omgk);

        // GEO satellite positions in intermediate frame
        double xgk = xkp * cosomgk - ykp * cosik * sinomgk;
        double ygk = xkp * sinomgk + ykp * cosik * cosomgk;
        double zgk = ykp * sinik;

        // Earth rotation during signal transmission
        double sinok = sin(omge * tk);
        double cosok = cos(omge * tk);

        // Transform to ECEF coordinates
        if (pos) {
            pos[0] =  xgk * cosok + ygk * sinok * COS_5 + zgk * sinok * SIN_5;
            pos[1] = -xgk * sinok + ygk * cosok * COS_5 + zgk * cosok * SIN_5;
            pos[2] = -ygk * SIN_5 + zgk * COS_5;
        }
    }
    else {
        // Standard satellite coordinate transformation

        // Correct longitude of ascending node
        double omgk = eph->OMG0 + (eph->OMGd - omge) * tk - omge * eph->toes;
        double sinomgk = sin(omgk);
        double cosomgk = cos(omgk);

        // Transform to ECEF coordinates
        if (pos) {
            pos[0] = xkp * cosomgk - ykp * cosik * sinomgk;
            pos[1] = xkp * sinomgk + ykp * cosik * cosomgk;
            pos[2] = ykp * sinik;
        }
    }

    // Satellite clock bias correction
    if (clk) {
        // Time from clock reference epoch
        double tk = time - eph->toc;

        // Clock polynomial correction
        *clk = eph->af0 + eph->af1 * tk + eph->af2 * tk * tk;

        // Relativistic correction
        *clk -= 2.0 * sqrt(mu * eph->A) * eph->e * sinEk / SQR(C_LIGHT);
    }

    // Position and clock error variance
    if (var) {
        *var = Idx2Ura(eph->sva);
        if (*var < 0.0) return 0;

        *var = SQR(*var);
    }

    // Return success
    return 1;
}

// GLONASS orbit differential equations (central force, J2, Earth rotation + external acc)
static void GloDeq(const double *x, double *xdot, const double *acc)
{
    // Position vector magnitude
    double r2 = x[0]*x[0] + x[1]*x[1] + x[2]*x[2];
    double r3 = r2 * sqrt(r2);

    // Check for singularity
    if (r2 <= 0.0) {
        for (int i = 0; i < 6; i++) xdot[i] = 0.0;
        return;
    }

    // Earth rotation rate squared
    double OMGE2 = SQR(OMGE_GLO);

    // J2 perturbation coefficients
    double a = 1.5 * J2_GLO * MU_GLO * SQR(RE_GLO) / r2 / r3;
    double b = 5.0 * x[2] * x[2] / r2;
    double c = -MU_GLO / r3 - a * (1.0 - b);

    // Set derivatives: position derivatives are velocities
    xdot[0] = x[3]; xdot[1] = x[4]; xdot[2] = x[5];

    // Set velocity derivatives (accelerations) including Earth rotation and J2
    xdot[3] = (c + OMGE2) * x[0] + 2.0 * OMGE_GLO * x[4] + acc[0];
    xdot[4] = (c + OMGE2) * x[1] - 2.0 * OMGE_GLO * x[3] + acc[1];
    xdot[5] = (c - 2.0 * a) * x[2] + acc[2];
}

// Runge-Kutta 4th order numerical integration
static void GloRK4(double dt, double *x, const double *acc)
{
    double k1[6], k2[6], k3[6], k4[6], w[6];

    // First stage
    GloDeq(x, k1, acc);
    for (int i = 0; i < 6; i++) w[i] = x[i] + k1[i] * dt / 2.0;

    // Second stage
    GloDeq(w, k2, acc);
    for (int i = 0; i < 6; i++) w[i] = x[i] + k2[i] * dt / 2.0;

    // Third stage
    GloDeq(w, k3, acc);
    for (int i = 0; i < 6; i++) w[i] = x[i] + k3[i] * dt;

    // Fourth stage
    GloDeq(w, k4, acc);

    // Update state vector
    for (int i = 0; i < 6; i++) {
        x[i] += (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]) * dt / 6.0;
    }
}

// Compute satellite position and clock bias of GLONASS satellite (1:ok, 0:error)
static int GloEph2Pos(double time, const eph_t *eph, double *pos, double *clk, double *var)
{
    // Initialize output
    if (pos) {pos[0] = pos[1] = pos[2] = 0.0;}
    if (clk) {*clk = 0.0;}
    if (var) {*var = 0.0;}

    // Check if the GLONASS ephemeris data is valid
    if (!eph) return 0;

    // Check if the satellite system is GLONASS
    int sys = Sat2Prn(eph->sat, NULL);
    if (sys != STR_GLO) return 0;

    // Time from ephemeris reference epoch
    double tk = time - eph->toe;

    // Initialize state vector [position, velocity]
    double x[6];
    for (int i = 0; i < 3; i++) {
        x[i]   = eph->pos[i];  // Initial position [m]
        x[i+3] = eph->vel[i];  // Initial velocity [m/s]
    }

    // Numerical integration using Runge-Kutta 4th order method
    double dt = (tk < 0.0) ? -TSTEP : TSTEP;
    for (double t = tk; fabs(t) > 1E-9; t -= dt) {
        // Adjust step size for final integration
        if (fabs(t) < TSTEP) dt = t;

        // Integrate one step with broadcast ephemeris acceleration (lunar-solar)
        GloRK4(dt, x, eph->acc);
    }

    // Set satellite position
    if (pos) {
        for (int i = 0; i < 3; i++) pos[i] = x[i];
    }

    // Satellite clock bias correction
    if (clk) {
        // GLONASS clock model: tau_n - gamma_n * (t - t_c)
        *clk = -eph->taun + eph->gamn * (time - eph->toc);
    }

    // Position and clock error variance
    if (var) {
        *var = SQR(STD_EPH_GLO);
    }

    // Return success
    return 1;
}

// Compute satellite position and clock bias of SBAS satellite (1:ok, 0:error)
static int SbsEph2Pos(double time, const eph_t *eph, double *pos, double *clk, double *var)
{
    // Initialize output
    if (pos) {pos[0] = pos[1] = pos[2] = 0.0;}
    if (clk) {*clk = 0.0;}
    if (var) {*var = 0.0;}

    // Time from ephemeris reference epoch
    double tk = time - eph->toe;

    // Satellite position
    if (pos) {
        for (int i = 0; i < 3; i++) {
            pos[i] = eph->pos[i] + eph->vel[i] * tk + 0.5 * eph->acc[i] * tk * tk;
        }
    }

    // Satellite clock bias correction
    if (clk) {
        *clk = eph->af0 + eph->af1 * tk;
    }

    // Position and clock error variance
    if (var) {
        *var = Idx2Ura(eph->sva);
        if (*var < 0.0) return 0;

        *var = SQR(*var);
    }

    // Return success
    return 1;
}

// Resize ephemeris data set structure
static int ResizeEphs(ephs_t *ephs, int nnew)
{
    // Check if the ephemeris data set structure is valid
    if (!ephs || nnew <= 0) return 0;

    eph_t *newEph = (eph_t *)realloc(ephs->eph, nnew * sizeof(eph_t));
    if (!newEph) return 0;

    ephs->eph = newEph;
    ephs->nmax = nnew;

    return 1;
}

// Compare ephemeris data by the order of satellite index and time transmission
// (eph.ttr)
static int CompareEph(const void *a, const void *b)
{
    const eph_t *eph1 = (const eph_t *)a;
    const eph_t *eph2 = (const eph_t *)b;

    // Satellite index
    if (eph1->sat != eph2->sat) return eph1->sat - eph2->sat;

    // Time transmission
    return (eph1->ttr > eph2->ttr) ? 1 : -1;
}

// =============================================================================
// Ephemeris data structure functions
// =============================================================================

// Initialize ephemeris data set structure
void InitEphs(ephs_t *ephs)
{
    // Check if the ephemeris data set structure is valid
    if (!ephs) return;

    // Initialize the ephemeris data set structure
    ephs->n = 0;
    ephs->nmax = 0;
    ephs->eph = NULL;
}

// Free ephemeris data set structure
void FreeEphs(ephs_t *ephs)
{
    // Check if the ephemeris data set structure is valid
    if (!ephs) return;

    // Free the ephemeris data set structure
    if (ephs->eph) {
        free(ephs->eph);
        ephs->eph = NULL;
    }
    ephs->n = ephs->nmax = 0;
}

// Add ephemeris data to ephemeris data set structure
int AddEph(ephs_t *ephs, const eph_t *eph)
{
    // Check if the ephemeris data set structure is valid
    if (!ephs || !eph) return 0;

    // Check if the ephemeris data set is full
    if (ephs->n >= ephs->nmax) {
        int nnew = (ephs->nmax == 0) ? 2 : ephs->nmax * 2;
        if (!ResizeEphs(ephs, nnew)) return 0;
    }

    // Add the ephemeris data to the ephemeris data set
    ephs->eph[ephs->n] = *eph;
    ephs->n++;

    return 1;
}

// Sort ephemeris data set by the order of satellite index and time transmission
// (eph.ttr)
void SortEphs(ephs_t *ephs)
{
    // Check if the ephemeris data set structure is valid
    if (!ephs || ephs->n == 0) return;

    // Sort the ephemeris data set by the order of satellite index and time transmission
    qsort(ephs->eph, ephs->n, sizeof(eph_t), CompareEph);

    // Delete duplicate ephemeris data (sat, iode, data, toe)
    int n = 0;
    for (int i = 0; i < ephs->n; i++) {

        // Check if the ephemeris data is the same as previous one
        if (i > 0 &&
            ephs->eph[i].sat  == ephs->eph[i-1].sat  &&
            ephs->eph[i].IODE == ephs->eph[i-1].IODE &&
            ephs->eph[i].data == ephs->eph[i-1].data &&
            ephs->eph[i].toe  == ephs->eph[i-1].toe) {
            continue;  // Skip duplicate
        }

        // Copy the ephemeris data to the next position
        if (n != i) {
            ephs->eph[n] = ephs->eph[i];
        }
        n++;
    }

    // Update the number of ephemeris data
    ephs->n = n;
}

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
    if (Sys2Str(sys) == STR_GAL) {
        if (type < 0 || type > 1) return; // GAL: 0 (I/NAV) or 1 (F/NAV)
    }
    else {
        if (type != 0) return;            // Others: 0 only
    }

    // Set broadcast ephemeris type
    EPHTYPE[sys-1] = type;
}

// Convert URA error value to index
int Ura2Idx(double err)
{
    if (err < 0.0) return -1;

    // Convert error value to index
    for (int i = 0; i < NURA; i++) {
        if (err <= URA_ERR[i]) return i;
    }

    // Return error if not found
    return -1;
}

// Convert SISA error value to index
int Sisa2Idx(double err)
{
    if (err < 0.0) return -1;

    // Convert error value to index
    if (err >= 0.0 && err <= 0.5) return (int)((err - 0.0) / 0.01) +  00;
    if (err >  0.5 && err <= 1.0) return (int)((err - 0.5) / 0.02) +  50;
    if (err >  1.0 && err <= 2.0) return (int)((err - 1.0) / 0.04) +  75;
    if (err >  2.0 && err <= 6.0) return (int)((err - 2.0) / 0.16) + 100;

    // Return error
    return -1;
}

// Convert URA index to error value [m]
double Idx2Ura(int ura)
{
    // Check if the URA index is valid
    if (ura < 0 || ura >= NURA) return -1.0;

    // Return error value
    return URA_ERR[ura];
}

// Convert SISA index to error value [m]
double Idx2Sisa(int sisa)
{
    // Convert SISA index to error value
    if (sisa <  0  ) return -1.0;
    if (sisa <= 49 ) return (sisa - 0  ) * 0.01 + 0.0;
    if (sisa <= 74 ) return (sisa - 50 ) * 0.02 + 0.5;
    if (sisa <= 99 ) return (sisa - 75 ) * 0.04 + 1.0;
    if (sisa <= 125) return (sisa - 100) * 0.16 + 2.0;

    // Return error
    return -1.0;
}

// Test broadcast ephemeris data
int TestEph(const eph_t *eph)
{
    // Check if the broadcast ephemeris data is valid
    if (!eph) return 0;

    // Check if the broadcast ephemeris satellite system is valid
    int sys = Sat2Prn(eph->sat, NULL);
    if (sys <= 0 || sys > NSYS) return 0;

    // Check if the broadcast ephemeris health is valid
    if (eph->svh == -1) return 0;
    switch (Sys2Str(sys)) {
        case STR_GAL: {
            // Check SISA value
            double err = Idx2Sisa(eph->sva);
            if (err < 0.0 || err > MAX_ERR_EPH) return 0;

            // Check health flag (E1-B DVS/HS, E5a DVS/HS, E5b DVS/HS)
            if (eph->svh & 0b111110111) return 0;
        } break;

        case STR_QZS: {
            // Check URA value
            double err = Idx2Ura(eph->sva);
            if (err < 0.0 || err > MAX_ERR_EPH) return 0;

            // Check health flag
            if (eph->svh & 0b101110) return 0;
        } break;

        default: {
            // Check URA value
            double err = Idx2Ura(eph->sva);
            if (err < 0.0 || err > MAX_ERR_EPH) return 0;

            // Check health flag
            if (eph->svh) return 0;
        } break;
    }

    // Return success
    return 1;
}

// Select broadcast ephemeris data
eph_t *SelectEph(double ephtime, int sat, const nav_t *nav, int iode)
{
    // Check if the ephemeris time is valid
    if (ephtime < 0.0) return NULL;

    // Check if the navigation data is valid
    if (!nav) return NULL;

    // Check if the satellite system is valid
    int sys = Sat2Prn(sat, NULL);
    if (sys <= 0 || sys > NSYS) return NULL;

    // Maximum difference Toe
    double maxdtoe;
    switch (Sys2Str(sys)) {
        case STR_GPS: maxdtoe = MAX_DTOE_GPS; break;
        case STR_GLO: maxdtoe = MAX_DTOE_GLO; break;
        case STR_GAL: maxdtoe = MAX_DTOE_GAL; break;
        case STR_BDS: maxdtoe = MAX_DTOE_BDS; break;
        case STR_QZS: maxdtoe = MAX_DTOE_QZS; break;
        case STR_IRN: maxdtoe = MAX_DTOE_IRN; break;
        case STR_SBS: maxdtoe = MAX_DTOE_SBS; break;
        default: return NULL;
    }

    // Select broadcast ephemeris data
    int    idx  = -1;
    double dttr = 0.0;
    for (int i = 0; i < nav->ephs[sat-1].n; i++) {

        // Get broadcast ephemeris data
        const eph_t *eph = nav->ephs[sat-1].eph + i;

        // Check if the satellite index is valid
        if (eph->sat != sat) continue;

        // Check if the IODE is valid
        if (iode >= 0 && eph->IODE != iode) continue;

        // Check ephemeris type for Galileo
        if (Sys2Str(sys) == STR_GAL) {
            if (GetEphType(sys) == 1) {
                // F/NAV
                if (!(eph->data & (1<<8))) continue;
            }
            else {
                // I/NAV
                if (!(eph->data & (1<<9))) continue;
            }
        }

        // Delete future navigation message
        if (eph->ttr > ephtime) continue;

        // Calculate difference between Toe and ephemeris time
        if (fabs(ephtime - eph->toe) > maxdtoe) continue;

        // Select closest ttr ephemeris data
        if (idx < 0 || fabs(eph->ttr - ephtime) < dttr) {
            idx  = i;
            dttr = fabs(eph->ttr - ephtime);
        }
    }

    // Return selected ephemeris data
    if (idx < 0) return NULL;
    return nav->ephs[sat-1].eph + idx;
}

// Compute satellite position, velocity, and clock bias/drift using broadcast
// ephemeris data
int SatPosClkBrdc(double ephtime, double time, int sat, const nav_t *nav,
int iode, mat_t *rs, mat_t *dts, double *var, eph_t *eph)
{
    // Check matrix dimensions
    if (rs ) {if ( rs->rows != 1 ||  rs->cols != 6) return 0;}
    if (dts) {if (dts->rows != 1 || dts->cols != 2) return 0;}

    // Initialize output
    if (rs ) {for (int i = 0; i < 6; i++) MatSetD(rs , 0, i, 0.0);}
    if (dts) {for (int i = 0; i < 2; i++) MatSetD(dts, 0, i, 0.0);}
    if (var) {*var = 0.0;}
    if (eph) {*eph = (eph_t){.svh = -1};}

    // Check if the navigation data is valid
    if (!nav) return 0;

    // Check if the satellite system is valid
    int sys = Sat2Prn(sat, NULL);
    if (sys <= 0 || sys > NSYS) return 0;

    // Check if the epehemris time and emission time are valid
    if (ephtime < 0.0 || time < 0.0) return 0;

    // Select broadcast ephemeris data
    eph_t *ephSelected = SelectEph(ephtime, sat, nav, iode);
    if (!ephSelected) return 0;

    // Compute satellite position and clock bias
    double pos0[3], posf[3], posb[3], clk0, clkf, clkb, var0, tt = 1E-3;

    switch (Sys2Str(sys)) {
        case STR_GPS:
        case STR_GAL:
        case STR_BDS:
        case STR_QZS:
        case STR_IRN: {
            if (!Eph2Pos(time     , ephSelected, pos0, &clk0, &var0)) return 0;
            if (!Eph2Pos(time + tt, ephSelected, posf, &clkf, NULL )) return 0;
            if (!Eph2Pos(time - tt, ephSelected, posb, &clkb, NULL )) return 0;
        } break;

        case STR_GLO: {
            if (!GloEph2Pos(time     , ephSelected, pos0, &clk0, &var0)) return 0;
            if (!GloEph2Pos(time + tt, ephSelected, posf, &clkf, NULL )) return 0;
            if (!GloEph2Pos(time - tt, ephSelected, posb, &clkb, NULL )) return 0;
        } break;

        case STR_SBS: {
            if (!SbsEph2Pos(time     , ephSelected, pos0, &clk0, &var0)) return 0;
            if (!SbsEph2Pos(time + tt, ephSelected, posf, &clkf, NULL )) return 0;
            if (!SbsEph2Pos(time - tt, ephSelected, posb, &clkb, NULL )) return 0;
        } break;

        default: return 0;
    }

    // Set satellite position and velocity
    if (rs) {
        for (int i = 0; i < 3; i++) MatSetD(rs, 0, i, pos0[i]);
        for (int i = 3; i < 6; i++) MatSetD(rs, 0, i, (posf[i-3] - posb[i-3]) / (2.0 * tt));
    }

    // Set satellite clock bias and drift
    if (dts) {
        MatSetD(dts, 0, 0, clk0);
        MatSetD(dts, 0, 1, (clkf - clkb) / (2.0 * tt));
    }

    // Set variance of position and clock bias
    if (var) *var = var0;

    // Set used broadcast ephemeris data
    if (eph) *eph = *ephSelected;

    // Return success
    return 1;
}

// =============================================================================
// End of file
// =============================================================================
