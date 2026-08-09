#ifndef RANKER_H
#define RANKER_H

#include "hirelens.h"

/* Sorts `results` (length `count`) in descending order of overall_score
 * using qsort, then assigns 1-based rank fields. */
void rank_candidates(CandidateResult *results, int count);

#endif /* RANKER_H */
