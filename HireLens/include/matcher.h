#ifndef MATCHER_H
#define MATCHER_H

#include "hirelens.h"

/* Runs skill matching (required + preferred, with synonym awareness),
 * education matching, keyword matching and project-evidence matching
 * between `jd` and the resume held inside `result`, populating all
 * matched/missing lists and the four raw sub-scores. Does NOT compute
 * the final weighted overall_score (see scorer.h for that). */
void run_matching(const JobDescription *jd, CandidateResult *result);

#endif /* MATCHER_H */
