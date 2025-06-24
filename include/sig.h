#ifndef SIG_H
#define SIG_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Signal data management functions
// =============================================================================

// -----------------------------------------------------------------------------
// Signal data structure management
// -----------------------------------------------------------------------------
void InitSigs(sigs_t *sigs);
void FreeSigs(sigs_t *sigs);
int  AddSig(sigs_t *sigs, const sig_t *sig);
void SortSigs(sigs_t *sigs);
int  FindSig(const sigs_t *sigs, double time, int rcv, int sat);

#ifdef __cplusplus
}
#endif

#endif // SIG_H
