/* ============================================================
 * scorer.c
 * Implements the configurable weighted scoring model defined in
 * include/config.h. Combines the raw sub-scores produced by the
 * matching engine into a single explainable overall_score.
 * ============================================================ */
#include "scorer.h"
#include "config.h"

static double pct(int part, int whole) {
    if (whole <= 0) return 100.0; /* nothing required in this category -> full marks */
    return (part / (double)whole) * 100.0;
}

void compute_scores(const JobDescription *jd, CandidateResult *result) {
    double required_pct = pct(result->matched_required_count, jd->required_count);
    double preferred_pct = pct(result->matched_preferred_count, jd->preferred_count);

    result->skill_score = (required_pct * REQUIRED_SKILL_WEIGHT) +
                           (preferred_pct * PREFERRED_SKILL_WEIGHT);

    result->overall_score =
        (result->skill_score      * WEIGHT_SKILLS) +
        (result->experience_score * WEIGHT_EXPERIENCE) +
        (result->education_score  * WEIGHT_EDUCATION) +
        (result->project_score    * WEIGHT_PROJECTS) +
        (result->keyword_score    * WEIGHT_KEYWORDS);

    if (result->overall_score > 100.0) result->overall_score = 100.0;
    if (result->overall_score < 0.0) result->overall_score = 0.0;
}
