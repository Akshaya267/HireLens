#ifndef SCORER_H
#define SCORER_H

#include "hirelens.h"

/* Computes result->skill_score from matched/missing required + preferred
 * skill counts, then computes result->overall_score as the configured
 * weighted sum of skill/experience/education/project/keyword scores.
 * Must be called AFTER run_matching(). */
void compute_scores(const JobDescription *jd, CandidateResult *result);

#endif /* SCORER_H */
