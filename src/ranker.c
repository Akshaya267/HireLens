/* ============================================================
 * ranker.c
 * Candidate ranking engine: sorts by overall_score descending
 * (ties broken by skill_score, then experience_score) and assigns
 * 1-based ranks.
 * ============================================================ */
#include <stdlib.h>
#include "ranker.h"

static int compare_candidates(const void *a, const void *b) {
    const CandidateResult *ca = (const CandidateResult *)a;
    const CandidateResult *cb = (const CandidateResult *)b;

    if (cb->overall_score > ca->overall_score) return 1;
    if (cb->overall_score < ca->overall_score) return -1;

    if (cb->skill_score > ca->skill_score) return 1;
    if (cb->skill_score < ca->skill_score) return -1;

    if (cb->experience_score > ca->experience_score) return 1;
    if (cb->experience_score < ca->experience_score) return -1;

    return 0;
}

void rank_candidates(CandidateResult *results, int count) {
    qsort(results, (size_t)count, sizeof(CandidateResult), compare_candidates);
    for (int i = 0; i < count; i++) {
        results[i].rank = i + 1;
    }
}
