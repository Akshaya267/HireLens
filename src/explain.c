/* ============================================================
 * explain.c
 * Explainability + recommendation engine. Translates raw scores
 * and matched/missing skill lists into plain-English strengths,
 * gaps, a suitability status, and a final recommendation string --
 * so every score is traceable back to a concrete reason.
 * ============================================================ */
#include <stdio.h>
#include <string.h>
#include "explain.h"
#include "config.h"
#include "text_utils.h"

static void add_note(char arr[][MAX_NOTE_LEN], int *count, const char *fmt_text) {
    if (*count >= MAX_NOTES) return;
    safe_strcpy(arr[*count], fmt_text, MAX_NOTE_LEN);
    (*count)++;
}

static const char *status_for_score(double score) {
    if (score >= THRESHOLD_HIGHLY_SUITABLE) return "Highly Suitable";
    if (score >= THRESHOLD_SUITABLE) return "Suitable";
    if (score >= THRESHOLD_PARTIALLY_SUITABLE) return "Partially Suitable";
    return "Not Suitable";
}

void generate_explanation(const JobDescription *jd, CandidateResult *result) {
    result->strengths_count = 0;
    result->gaps_count = 0;

    char buf[MAX_NOTE_LEN];

    /* --- Strengths --- */
    if (jd->required_count > 0) {
        snprintf(buf, sizeof(buf), "Matched %d of %d required skills (%.0f%%)",
                 result->matched_required_count, jd->required_count,
                 (result->matched_required_count / (double)jd->required_count) * 100.0);
        add_note(result->strengths, &result->strengths_count, buf);
    }
    if (result->matched_preferred_count > 0) {
        snprintf(buf, sizeof(buf), "Also brings %d preferred/bonus skill(s)", result->matched_preferred_count);
        add_note(result->strengths, &result->strengths_count, buf);
    }
    if (result->experience_score >= 100.0 && jd->min_experience_years > 0) {
        snprintf(buf, sizeof(buf), "Meets or exceeds the %d year experience requirement",
                 jd->min_experience_years);
        add_note(result->strengths, &result->strengths_count, buf);
    }
    if (result->education_score >= 100.0 && jd->education_count > 0) {
        add_note(result->strengths, &result->strengths_count, "Education matches the JD requirement");
    }
    if (result->project_score >= 60.0) {
        add_note(result->strengths, &result->strengths_count,
                 "Projects provide strong practical evidence of required/preferred skills");
    }
    if (result->keyword_score >= 60.0 && jd->keyword_count > 0) {
        add_note(result->strengths, &result->strengths_count,
                 "Resume language strongly reflects the JD's domain keywords");
    }
    /* Highlight synonym-based matches as a strength of the analysis */
    int synonym_hits = 0;
    for (int i = 0; i < result->matched_required_count; i++) {
        if (result->matched_required[i].via_synonym) synonym_hits++;
    }
    for (int i = 0; i < result->matched_preferred_count; i++) {
        if (result->matched_preferred[i].via_synonym) synonym_hits++;
    }
    if (synonym_hits > 0) {
        snprintf(buf, sizeof(buf), "%d skill(s) recognized through synonym mapping (e.g. abbreviations)",
                 synonym_hits);
        add_note(result->strengths, &result->strengths_count, buf);
    }
    if (result->strengths_count == 0) {
        add_note(result->strengths, &result->strengths_count, "No standout strengths identified for this role");
    }

    /* --- Gaps --- */
    if (result->missing_required_count > 0) {
        snprintf(buf, sizeof(buf), "Missing %d required skill(s): ", result->missing_required_count);
        int remaining = (int)(MAX_NOTE_LEN - strlen(buf) - 1);
        for (int i = 0; i < result->missing_required_count && remaining > 0; i++) {
            char piece[160];
            snprintf(piece, sizeof(piece), "%s%s", result->missing_required[i],
                     (i < result->missing_required_count - 1) ? ", " : "");
            strncat(buf, piece, (size_t)remaining);
            remaining = (int)(MAX_NOTE_LEN - strlen(buf) - 1);
        }
        add_note(result->gaps, &result->gaps_count, buf);
    }
    if (jd->min_experience_years > 0 && result->resume.experience_years < jd->min_experience_years) {
        snprintf(buf, sizeof(buf), "Experience is %d year(s), below the required %d year(s)",
                 result->resume.experience_years, jd->min_experience_years);
        add_note(result->gaps, &result->gaps_count, buf);
    }
    if (result->education_score < 100.0 && jd->education_count > 0) {
        add_note(result->gaps, &result->gaps_count, "Education background does not clearly match the JD requirement");
    }
    if (result->project_score < 40.0) {
        add_note(result->gaps, &result->gaps_count, "Projects show limited evidence of the required/preferred skills");
    }
    if (result->missing_preferred_count > 0) {
        snprintf(buf, sizeof(buf), "Missing %d preferred/bonus skill(s)", result->missing_preferred_count);
        add_note(result->gaps, &result->gaps_count, buf);
    }
    if (result->gaps_count == 0) {
        add_note(result->gaps, &result->gaps_count, "No significant gaps identified for this role");
    }

    /* --- Status + Recommendation --- */
    safe_strcpy(result->status, status_for_score(result->overall_score), sizeof(result->status));

    const char *action;
    if (result->overall_score >= THRESHOLD_HIGHLY_SUITABLE) {
        action = "Recommended for interview -- strong alignment with the role.";
    } else if (result->overall_score >= THRESHOLD_SUITABLE) {
        action = "Recommended for interview with minor gaps to probe during screening.";
    } else if (result->overall_score >= THRESHOLD_PARTIALLY_SUITABLE) {
        action = "Consider only if the candidate pool is limited; several gaps exist.";
    } else {
        action = "Not recommended for this specific role based on current profile.";
    }

    snprintf(result->recommendation, sizeof(result->recommendation),
             "%s scored %.1f/100 overall (%s). %s Required skills matched: %d/%d. "
             "Experience: %d yr(s) vs %d required.",
             result->resume.name, result->overall_score, result->status, action,
             result->matched_required_count, jd->required_count,
             result->resume.experience_years, jd->min_experience_years);
}
