// =============================================================================
// Signal Data Management Module
//
// MATLAB Sig() 함수를 참고한 C 구현
// Signal data table creation and management functions
//
// Author: Yongrae Jo
// =============================================================================

// Standard library
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// GNSS library
#include "types.h"          // for sig_t, sigs_t
#include "const.h"          // for NFREQ constants
#include "sig.h"            // for signal functions

// =============================================================================
// Signal data structure management functions
// =============================================================================

// -----------------------------------------------------------------------------
// Initialize signal data set
// -----------------------------------------------------------------------------
void InitSigs(sigs_t *sigs) {
    if (!sigs) return;

    sigs->n = 0;
    sigs->nmax = 0;
    sigs->sig = NULL;
}

// -----------------------------------------------------------------------------
// Free signal data set
// -----------------------------------------------------------------------------
void FreeSigs(sigs_t *sigs) {
    if (!sigs) return;

    if (sigs->sig) {
        free(sigs->sig);
        sigs->sig = NULL;
    }
    sigs->n = sigs->nmax = 0;
}

// -----------------------------------------------------------------------------
// Add signal to signal data set
// -----------------------------------------------------------------------------
int AddSig(sigs_t *sigs, const sig_t *sig) {
    if (!sigs || !sig) return 0;

    // Expand array if necessary
    if (sigs->n >= sigs->nmax) {
        int nnew = (sigs->nmax == 0) ? 2 : sigs->nmax * 2;
        sig_t *newsig = (sig_t *)realloc(sigs->sig, nnew * sizeof(sig_t));
        if (!newsig) return 0;

        sigs->sig = newsig;
        sigs->nmax = nnew;
    }

    // Copy signal data
    sigs->sig[sigs->n] = *sig;
    sigs->n++;

    return 1;
}

// -----------------------------------------------------------------------------
// Sort signals by time, receiver, satellite
// -----------------------------------------------------------------------------
static int CompareSig(const void *a, const void *b) {
    const sig_t *sig1 = (const sig_t *)a;
    const sig_t *sig2 = (const sig_t *)b;

    // Sort by time first
    if (sig1->time < sig2->time) return -1;
    if (sig1->time > sig2->time) return +1;

    // Then by receiver
    if (sig1->rcv < sig2->rcv) return -1;
    if (sig1->rcv > sig2->rcv) return +1;

    // Finally by satellite
    if (sig1->sat < sig2->sat) return -1;
    if (sig1->sat > sig2->sat) return +1;

    return 0;
}

void SortSigs(sigs_t *sigs) {
    if (!sigs || !sigs->sig || sigs->n <= 1) return;

    qsort(sigs->sig, sigs->n, sizeof(sig_t), CompareSig);
}

// =============================================================================
// Signal data access and utility functions
// =============================================================================

// -----------------------------------------------------------------------------
// Find signal by time, receiver, satellite
// -----------------------------------------------------------------------------
int FindSig(const sigs_t *sigs, double time, int rcv, int sat) {
    if (!sigs || !sigs->sig) return -1;

    for (int i = 0; i < sigs->n; i++) {
        if (fabs(sigs->sig[i].time - time) < 1e-3 &&  // 1ms tolerance
            sigs->sig[i].rcv == rcv &&
            sigs->sig[i].sat == sat) {
            return i;
        }
    }

    return -1;  // Not found
}
