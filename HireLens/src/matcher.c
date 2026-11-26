/* ============================================================
 * matcher.c
 * The core matching engine. For a given (JobDescription, Resume)
 * pair this module determines:
 *   - which required/preferred skills are present (directly or via
 *     synonym normalization) and which are missing
 *   - an education match percentage
 *   - a project-evidence score (do the candidate's projects actually
 *     mention the required/preferred skills?)
 *   - a keyword match percentage (soft-skill / domain terms from the JD)
 *
 * All matching is done with plain C string operations (strcmp,
 * strstr via str_contains_ci) -- no external NLP libraries.
 * ============================================================ */
#include <stdio.h>
#include <string.h>
#include "matcher.h"
#include "text_utils.h"
#include "synonyms.h"

/* Finds `canonical_skill` inside the resume's skill list.
 * Returns 1 if found, and sets *via_synonym to 1 if the matching
 * resume skill's raw text differs from the JD skill's raw text
 * (i.e. the match only worked because of synonym normalization). */
static int find_skill_in_resume(const Resume *resume, const Skill *jd_skill, int *via_synonym) {
    for (int i = 0; i < resume->skill_count; i++) {
        if (strcmp(resume->skills[i].canonical, jd_skill->canonical) == 0) {
            *via_synonym = !str_equals_ci(resume->skills[i].raw, jd_skill->raw);
            return 1;
        }
    }
    return 0;
}

static void match_skill_group(const Skill *jd_list, int jd_count, const Resume *resume,
                               MatchedSkill *matched_out, int *matched_count,
                               char missing_out[][MAX_SKILL_LEN], int *missing_count) {
    *matched_count = 0;
    *missing_count = 0;
    for (int i = 0; i < jd_count; i++) {
        int via_synonym = 0;
        if (find_skill_in_resume(resume, &jd_list[i], &via_synonym)) {
            safe_strcpy(matched_out[*matched_count].skill, jd_list[i].raw, MAX_SKILL_LEN);
            matched_out[*matched_count].via_synonym = via_synonym;
            (*matched_count)++;
        } else {
            safe_strcpy(missing_out[*missing_count], jd_list[i].raw, MAX_SKILL_LEN);
            (*missing_count)++;
        }
    }
}

/* NOTE: the EDUCATION list in a JD represents ACCEPTED ALTERNATIVES
 * (e.g. "B.Tech CS, BCA, B.E CS" means ANY one of these qualifies),
 * not a set of simultaneously-required degrees. So the candidate only
 * needs to satisfy ONE of the listed alternatives to score full marks. */
static double education_match_score(const JobDescription *jd, const Resume *resume) {
    if (jd->education_count == 0) return 100.0; /* nothing required -> full marks */
    if (resume->education_count == 0) return 0.0;

    for (int i = 0; i < jd->education_count; i++) {
        char jd_canon[MAX_SKILL_LEN];
        normalize_term(jd->education[i], jd_canon, MAX_SKILL_LEN);

        for (int r = 0; r < resume->education_count; r++) {
            char resume_canon[MAX_SKILL_LEN];
            normalize_term(resume->education[r], resume_canon, MAX_SKILL_LEN);

            if (strcmp(jd_canon, resume_canon) == 0 ||
                str_contains_ci(resume->education[r], jd->education[i]) ||
                str_contains_ci(resume_canon, jd_canon)) {
                return 100.0; /* any single alternative satisfied -> full marks */
            }
        }
    }
    return 0.0; /* none of the accepted alternatives were found */
}

static double experience_match_score(const JobDescription *jd, const Resume *resume) {
    if (jd->min_experience_years <= 0) return 100.0;
    if (resume->experience_years >= jd->min_experience_years) return 100.0;
    double score = (resume->experience_years / (double)jd->min_experience_years) * 100.0;
    if (score < 0) score = 0;
    return score;
}

/* Project evidence: what fraction of the combined required+preferred
 * skill set is actually mentioned inside the candidate's PROJECTS text?
 * This rewards candidates who applied a skill practically, not just
 * listed it. */
static double project_evidence_score(const JobDescription *jd, const Resume *resume) {
    int total = jd->required_count + jd->preferred_count;
    if (total == 0) return 100.0;
    if (strlen(resume->projects_text) == 0) return 0.0;

    int hits = 0;
    for (int i = 0; i < jd->required_count; i++) {
        if (str_contains_ci(resume->projects_text, jd->required_skills[i].canonical) ||
            str_contains_ci(resume->projects_text, jd->required_skills[i].raw)) {
            hits++;
        }
    }
    for (int i = 0; i < jd->preferred_count; i++) {
        if (str_contains_ci(resume->projects_text, jd->preferred_skills[i].canonical) ||
            str_contains_ci(resume->projects_text, jd->preferred_skills[i].raw)) {
            hits++;
        }
    }
    double score = (hits / (double)total) * 100.0;
    if (score > 100.0) score = 100.0;
    return score;
}

static double keyword_match_score(const JobDescription *jd, const Resume *resume) {
    if (jd->keyword_count == 0) return 100.0;

    char blob[MAX_TEXT * 2];
    snprintf(blob, sizeof(blob), "%s %s", resume->summary_text, resume->projects_text);

    int hits = 0;
    for (int i = 0; i < jd->keyword_count; i++) {
        if (str_contains_ci(blob, jd->keywords[i])) {
            hits++;
        }
    }
    return (hits / (double)jd->keyword_count) * 100.0;
}

void run_matching(const JobDescription *jd, CandidateResult *result) {
    const Resume *resume = &result->resume;

    match_skill_group(jd->required_skills, jd->required_count, resume,
                       result->matched_required, &result->matched_required_count,
                       result->missing_required, &result->missing_required_count);

    match_skill_group(jd->preferred_skills, jd->preferred_count, resume,
                       result->matched_preferred, &result->matched_preferred_count,
                       result->missing_preferred, &result->missing_preferred_count);

    result->education_score  = education_match_score(jd, resume);
    result->experience_score = experience_match_score(jd, resume);
    result->project_score    = project_evidence_score(jd, resume);
    result->keyword_score    = keyword_match_score(jd, resume);
    /* skill_score is computed in scorer.c, since it combines required
     * and preferred match percentages using the configurable weights */
}
