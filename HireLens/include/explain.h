#ifndef EXPLAIN_H
#define EXPLAIN_H

#include "hirelens.h"

/* Generates human readable strengths[], gaps[], status and
 * recommendation text for `result`, based on its already-computed
 * scores and matched/missing skill lists. Must be called AFTER
 * run_matching() and compute_scores(). */
void generate_explanation(const JobDescription *jd, CandidateResult *result);

#endif /* EXPLAIN_H */
